/*
 * Copyright (c) 2017-2023 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "arm_compute/core/CL/OpenCL.h"
#pragma GCC diagnostic pop

#include "arm_compute/core/Error.h"

#include <algorithm>
#include <dlfcn.h>
#include <iostream>
#include <sstream>

namespace arm_compute
{
CLSymbols::CLSymbols() noexcept(false) : _loaded({false, false})
{
}

CLSymbols &CLSymbols::get()
{
    static CLSymbols symbols;
    return symbols;
}

bool CLSymbols::load_default()
{
    static const std::vector<std::string> libraries_filenames{"libOpenCL.so", "libGLES_mali.so", "libmali.so"};

    if (_loaded.first)
    {
        return _loaded.second;
    }

    // Indicate that default loading has been tried
    _loaded.first = true;

    if (load(libraries_filenames, /* use_loader */ false))
    {
        ARM_COMPUTE_ERROR_ON_MSG(this->clBuildProgram_ptr == nullptr,
                                 "Failed to load OpenCL symbols from shared library");
        return true;
    }

#ifdef __ANDROID__
    // When running in NDK environment, the above libraries are not accessible.
    static const std::vector<std::string> android_libraries_filenames{"libOpenCL-pixel.so", "libOpenCL-car.so"};

    if (load(android_libraries_filenames, /* use_loader */ true))
    {
        ARM_COMPUTE_ERROR_ON_MSG(this->clBuildProgram_ptr == nullptr,
                                 "Failed to load OpenCL symbols from android shared library");
        return true;
    }
#endif // __ANDROID__

    // If not returned till here then libraries not found
    std::stringstream ss;
    std::for_each(libraries_filenames.begin(), libraries_filenames.end(),
                  [&ss](const std::string &s) { ss << s << " "; });
#ifdef __ANDROID__
    std::for_each(android_libraries_filenames.begin(), android_libraries_filenames.end(),
                  [&ss](const std::string &s) { ss << s << " "; });
#endif // __ANDROID__
    std::cerr << "Couldn't find any of the following OpenCL library: " << ss.str() << std::endl;
    return false;
}

bool CLSymbols::load(const std::vector<std::string> &libraries_filenames, bool use_loader)
{
    void        *handle = nullptr;
    unsigned int index  = 0;
    for (index = 0; index < libraries_filenames.size(); ++index)
    {
        handle = dlopen(libraries_filenames[index].c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (handle != nullptr)
        {
            break;
        }
    }
    if (index == libraries_filenames.size())
    {
        // Set status of loading to failed
        _loaded.second = false;
        return false;
    }

#ifdef __ANDROID__
    typedef void *(*loadOpenCLPointer_t)(const char *name);
    loadOpenCLPointer_t loadOpenCLPointer;
    if (use_loader)
    {
        typedef void (*enableOpenCL_t)();
        enableOpenCL_t enableOpenCL = reinterpret_cast<enableOpenCL_t>(dlsym(handle, "enableOpenCL"));
        enableOpenCL();

        loadOpenCLPointer = reinterpret_cast<loadOpenCLPointer_t>(dlsym(handle, "loadOpenCLPointer"));
    }
    else
    {
        loadOpenCLPointer = nullptr;
    }
#define LOAD_FUNCTION_PTR(func_name, _handle)                                                            \
    func_name##_ptr = reinterpret_cast<decltype(func_name) *>(use_loader ? loadOpenCLPointer(#func_name) \
                                                                         : dlsym(handle, #func_name));
#else /* __ANDROID__ */
    (void)use_loader; // Avoid unused warning
#define LOAD_FUNCTION_PTR(func_name, handle) \
    func_name##_ptr = reinterpret_cast<decltype(func_name) *>(dlsym(handle, #func_name));
#endif /* __ANDROID__ */

#define LOAD_EXTENSION_FUNCTION_PTR(func_name, platform_id) \
    func_name##_ptr =                                       \
        reinterpret_cast<decltype(func_name) *>(clGetExtensionFunctionAddressForPlatform(platform_id, #func_name));

    LOAD_FUNCTION_PTR(clCreateContext, handle);
    LOAD_FUNCTION_PTR(clCreateContextFromType, handle);
    LOAD_FUNCTION_PTR(clCreateCommandQueue, handle);
    LOAD_FUNCTION_PTR(clCreateCommandQueueWithProperties, handle);
    LOAD_FUNCTION_PTR(clGetContextInfo, handle);
    LOAD_FUNCTION_PTR(clBuildProgram, handle);
    LOAD_FUNCTION_PTR(clEnqueueNDRangeKernel, handle);
    LOAD_FUNCTION_PTR(clSetKernelArg, handle);
    LOAD_FUNCTION_PTR(clReleaseKernel, handle);
    LOAD_FUNCTION_PTR(clCreateProgramWithSource, handle);
    LOAD_FUNCTION_PTR(clCreateBuffer, handle);
    LOAD_FUNCTION_PTR(clRetainKernel, handle);
    LOAD_FUNCTION_PTR(clCreateKernel, handle);
    LOAD_FUNCTION_PTR(clGetProgramInfo, handle);
    LOAD_FUNCTION_PTR(clFlush, handle);
    LOAD_FUNCTION_PTR(clFinish, handle);
    LOAD_FUNCTION_PTR(clReleaseProgram, handle);
    LOAD_FUNCTION_PTR(clRetainContext, handle);
    LOAD_FUNCTION_PTR(clCreateProgramWithBinary, handle);
    LOAD_FUNCTION_PTR(clReleaseCommandQueue, handle);
    LOAD_FUNCTION_PTR(clEnqueueMapBuffer, handle);
    LOAD_FUNCTION_PTR(clRetainProgram, handle);
    LOAD_FUNCTION_PTR(clGetProgramBuildInfo, handle);
    LOAD_FUNCTION_PTR(clEnqueueReadBuffer, handle);
    LOAD_FUNCTION_PTR(clEnqueueWriteBuffer, handle);
    LOAD_FUNCTION_PTR(clReleaseEvent, handle);
    LOAD_FUNCTION_PTR(clReleaseContext, handle);
    LOAD_FUNCTION_PTR(clRetainCommandQueue, handle);
    LOAD_FUNCTION_PTR(clEnqueueUnmapMemObject, handle);
    LOAD_FUNCTION_PTR(clRetainMemObject, handle);
    LOAD_FUNCTION_PTR(clReleaseMemObject, handle);
    LOAD_FUNCTION_PTR(clGetDeviceInfo, handle);
    LOAD_FUNCTION_PTR(clGetDeviceIDs, handle);
    LOAD_FUNCTION_PTR(clGetMemObjectInfo, handle);
    LOAD_FUNCTION_PTR(clRetainEvent, handle);
    LOAD_FUNCTION_PTR(clGetPlatformInfo, handle);
    LOAD_FUNCTION_PTR(clGetPlatformIDs, handle);
    LOAD_FUNCTION_PTR(clGetKernelWorkGroupInfo, handle);
    LOAD_FUNCTION_PTR(clGetCommandQueueInfo, handle);
    LOAD_FUNCTION_PTR(clGetKernelInfo, handle);
    LOAD_FUNCTION_PTR(clGetEventProfilingInfo, handle);
    LOAD_FUNCTION_PTR(clSVMAlloc, handle);
    LOAD_FUNCTION_PTR(clSVMFree, handle);
    LOAD_FUNCTION_PTR(clEnqueueSVMMap, handle);
    LOAD_FUNCTION_PTR(clEnqueueSVMUnmap, handle);
    LOAD_FUNCTION_PTR(clEnqueueMarker, handle);
    LOAD_FUNCTION_PTR(clWaitForEvents, handle);
    LOAD_FUNCTION_PTR(clCreateImage, handle);
    LOAD_FUNCTION_PTR(clSetKernelExecInfo, handle);
    LOAD_FUNCTION_PTR(clGetExtensionFunctionAddressForPlatform, handle);

    // Load Extensions

    // Number of platforms is assumed to be 1. For this to be greater than 1,
    // the system must have more than one OpenCL implementation provided by
    // different vendors. This is not our use case. Besides, the library
    // already assumes one implementation as it uses one handle to load core
    // functions.
    constexpr unsigned int      num_platforms = 1U;
    std::vector<cl_platform_id> platform_ids(num_platforms);
    clGetPlatformIDs(num_platforms, platform_ids.data(), nullptr);

    // Command buffer and mutable dispatch command buffer extensions
    /// TODO: (COMPMID-6742) Load Command Buffer extensions in a Portable way
    /// using clGetExtensionFunctionAddressForPlatform().
    /// The details can be found here:
    ///    https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_Ext.html#getting-opencl-api-extension-function-pointers
    ///
    /// @note: There are some problems reported while loading these extensions in the recommended way.
    ///        For details, please see COMPUTE-16545
    LOAD_FUNCTION_PTR(clCreateCommandBufferKHR, handle);
    LOAD_FUNCTION_PTR(clRetainCommandBufferKHR, handle);
    LOAD_FUNCTION_PTR(clReleaseCommandBufferKHR, handle);
    LOAD_FUNCTION_PTR(clFinalizeCommandBufferKHR, handle);
    LOAD_FUNCTION_PTR(clEnqueueCommandBufferKHR, handle);
    LOAD_FUNCTION_PTR(clCommandNDRangeKernelKHR, handle);

    LOAD_FUNCTION_PTR(clUpdateMutableCommandsKHR, handle);

    // Third-party extensions
    LOAD_EXTENSION_FUNCTION_PTR(clImportMemoryARM, platform_ids[0]);

#undef LOAD_FUNCTION_PTR
#undef LOAD_EXTENSION_FUNCTION_PTR

    //Don't call dlclose(handle) or all the symbols will be unloaded !

    // Disable default loading and set status to successful
    _loaded = std::make_pair(true, true);

    return true;
}

bool opencl_is_available()
{
    CLSymbols::get().load_default();

    // Using static objects that rely on OpenCL in their constructor or
    // destructor is implementation defined according to the OpenCL API
    // Specification. These objects include CLScheduler.
    //
    // For compatibility with OpenCL runtimes that also use static objects to
    // hold their state, we call a harmless OpenCL function (clGetPlatformIDs
    // with invalid parameters must result in CL_INVALID_VALUE) to ensure the
    // runtimes have a chance to initialize their static objects first. Thanks
    // to C++11 rules about normal program completion (cf [basic.start]), this
    // ensures their static objects are destroyed last, i.e. after the
    // singleton CLScheduler is destroyed.
    //
    // When OpenCL is not available, this call results in CL_OUT_OF_RESOURCES,
    // which is equally harmless.
    (void)clGetPlatformIDs(0, nullptr, nullptr);

    return CLSymbols::get().clBuildProgram_ptr != nullptr;
}
} // namespace arm_compute

cl_int clEnqueueMarker(cl_command_queue command_queue, cl_event *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueMarker_ptr;
    if (func != nullptr)
    {
        return func(command_queue, event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clWaitForEvents(cl_uint num_events, const cl_event *event_list)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clWaitForEvents_ptr;
    if (func != nullptr)
    {
        return func(num_events, event_list);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clEnqueueSVMMap(cl_command_queue command_queue,
                       cl_bool          blocking_map,
                       cl_map_flags     flags,
                       void            *svm_ptr,
                       size_t           size,
                       cl_uint          num_events_in_wait_list,
                       const cl_event  *event_wait_list,
                       cl_event        *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueSVMMap_ptr;
    if (func != nullptr)
    {
        return func(command_queue, blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clEnqueueSVMUnmap(cl_command_queue command_queue,
                         void            *svm_ptr,
                         cl_uint          num_events_in_wait_list,
                         const cl_event  *event_wait_list,
                         cl_event        *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueSVMUnmap_ptr;
    if (func != nullptr)
    {
        return func(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

void *clSVMAlloc(cl_context context, cl_svm_mem_flags_arm flags, size_t size, cl_uint alignment)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clSVMAlloc_ptr;
    if (func != nullptr)
    {
        return func(context, flags, size, alignment);
    }
    else
    {
        return nullptr;
    }
}

void clSVMFree(cl_context context, void *svm_pointer)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clSVMFree_ptr;
    if (func != nullptr)
    {
        func(context, svm_pointer);
    }
}

cl_int clGetContextInfo(cl_context      context,
                        cl_context_info param_name,
                        size_t          param_value_size,
                        void           *param_value,
                        size_t         *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetContextInfo_ptr;
    if (func != nullptr)
    {
        return func(context, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_command_queue clCreateCommandQueue(cl_context                  context,
                                      cl_device_id                device,
                                      cl_command_queue_properties properties,
                                      cl_int                     *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateCommandQueue_ptr;
    if (func != nullptr)
    {
        return func(context, device, properties, errcode_ret);
    }
    else
    {
        return nullptr;
    }
}

cl_command_queue clCreateCommandQueueWithProperties(cl_context                 context,
                                                    cl_device_id               device,
                                                    const cl_queue_properties *properties,
                                                    cl_int                    *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateCommandQueueWithProperties_ptr;
    if (func != nullptr)
    {
        return func(context, device, properties, errcode_ret);
    }
    else
    {
        return nullptr;
    }
}

cl_context clCreateContext(const cl_context_properties *properties,
                           cl_uint                      num_devices,
                           const cl_device_id          *devices,
                           void (*pfn_notify)(const char *, const void *, size_t, void *),
                           void   *user_data,
                           cl_int *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateContext_ptr;
    if (func != nullptr)
    {
        return func(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    }
    else
    {
        return nullptr;
    }
}

cl_context clCreateContextFromType(const cl_context_properties *properties,
                                   cl_device_type               device_type,
                                   void (*pfn_notify)(const char *, const void *, size_t, void *),
                                   void   *user_data,
                                   cl_int *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateContextFromType_ptr;
    if (func != nullptr)
    {
        return func(properties, device_type, pfn_notify, user_data, errcode_ret);
    }
    else
    {
        return nullptr;
    }
}

cl_int clBuildProgram(cl_program          program,
                      cl_uint             num_devices,
                      const cl_device_id *device_list,
                      const char         *options,
                      void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                      void *user_data)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clBuildProgram_ptr;
    if (func != nullptr)
    {
        return func(program, num_devices, device_list, options, pfn_notify, user_data);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clEnqueueNDRangeKernel(cl_command_queue command_queue,
                              cl_kernel        kernel,
                              cl_uint          work_dim,
                              const size_t    *global_work_offset,
                              const size_t    *global_work_size,
                              const size_t    *local_work_size,
                              cl_uint          num_events_in_wait_list,
                              const cl_event  *event_wait_list,
                              cl_event        *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueNDRangeKernel_ptr;
    if (func != nullptr)
    {
        return func(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size,
                    num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clSetKernelArg_ptr;
    if (func != nullptr)
    {
        return func(kernel, arg_index, arg_size, arg_value);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clRetainMemObject(cl_mem memobj)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clRetainMemObject_ptr;
    if (func != nullptr)
    {
        return func(memobj);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clReleaseMemObject(cl_mem memobj)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clReleaseMemObject_ptr;
    if (func != nullptr)
    {
        return func(memobj);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clEnqueueUnmapMemObject(cl_command_queue command_queue,
                               cl_mem           memobj,
                               void            *mapped_ptr,
                               cl_uint          num_events_in_wait_list,
                               const cl_event  *event_wait_list,
                               cl_event        *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueUnmapMemObject_ptr;
    if (func != nullptr)
    {
        return func(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clRetainCommandQueue(cl_command_queue command_queue)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clRetainCommandQueue_ptr;
    if (func != nullptr)
    {
        return func(command_queue);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clReleaseContext(cl_context context)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clReleaseContext_ptr;
    if (func != nullptr)
    {
        return func(context);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}
cl_int clReleaseEvent(cl_event event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clReleaseEvent_ptr;
    if (func != nullptr)
    {
        return func(event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clEnqueueWriteBuffer(cl_command_queue command_queue,
                            cl_mem           buffer,
                            cl_bool          blocking_write,
                            size_t           offset,
                            size_t           size,
                            const void      *ptr,
                            cl_uint          num_events_in_wait_list,
                            const cl_event  *event_wait_list,
                            cl_event        *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueWriteBuffer_ptr;
    if (func != nullptr)
    {
        return func(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list,
                    event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clEnqueueReadBuffer(cl_command_queue command_queue,
                           cl_mem           buffer,
                           cl_bool          blocking_read,
                           size_t           offset,
                           size_t           size,
                           void            *ptr,
                           cl_uint          num_events_in_wait_list,
                           const cl_event  *event_wait_list,
                           cl_event        *event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueReadBuffer_ptr;
    if (func != nullptr)
    {
        return func(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list,
                    event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetProgramBuildInfo(cl_program            program,
                             cl_device_id          device,
                             cl_program_build_info param_name,
                             size_t                param_value_size,
                             void                 *param_value,
                             size_t               *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetProgramBuildInfo_ptr;
    if (func != nullptr)
    {
        return func(program, device, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clRetainProgram(cl_program program)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clRetainProgram_ptr;
    if (func != nullptr)
    {
        return func(program);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

void *clEnqueueMapBuffer(cl_command_queue command_queue,
                         cl_mem           buffer,
                         cl_bool          blocking_map,
                         cl_map_flags     map_flags,
                         size_t           offset,
                         size_t           size,
                         cl_uint          num_events_in_wait_list,
                         const cl_event  *event_wait_list,
                         cl_event        *event,
                         cl_int          *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clEnqueueMapBuffer_ptr;
    if (func != nullptr)
    {
        return func(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list,
                    event_wait_list, event, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}

cl_int clReleaseCommandQueue(cl_command_queue command_queue)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clReleaseCommandQueue_ptr;
    if (func != nullptr)
    {
        return func(command_queue);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_program clCreateProgramWithBinary(cl_context            context,
                                     cl_uint               num_devices,
                                     const cl_device_id   *device_list,
                                     const size_t         *lengths,
                                     const unsigned char **binaries,
                                     cl_int               *binary_status,
                                     cl_int               *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateProgramWithBinary_ptr;
    if (func != nullptr)
    {
        return func(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}

cl_int clRetainContext(cl_context context)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clRetainContext_ptr;
    if (func != nullptr)
    {
        return func(context);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clReleaseProgram(cl_program program)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clReleaseProgram_ptr;
    if (func != nullptr)
    {
        return func(program);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clFlush(cl_command_queue command_queue)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clFlush_ptr;
    if (func != nullptr)
    {
        return func(command_queue);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clFinish(cl_command_queue command_queue)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clFinish_ptr;
    if (func != nullptr)
    {
        return func(command_queue);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetProgramInfo(cl_program      program,
                        cl_program_info param_name,
                        size_t          param_value_size,
                        void           *param_value,
                        size_t         *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetProgramInfo_ptr;
    if (func != nullptr)
    {
        return func(program, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_kernel clCreateKernel(cl_program program, const char *kernel_name, cl_int *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateKernel_ptr;
    if (func != nullptr)
    {
        return func(program, kernel_name, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}

cl_int clRetainKernel(cl_kernel kernel)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clRetainKernel_ptr;
    if (func != nullptr)
    {
        return func(kernel);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_mem clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateBuffer_ptr;
    if (func != nullptr)
    {
        return func(context, flags, size, host_ptr, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}

cl_program clCreateProgramWithSource(
    cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateProgramWithSource_ptr;
    if (func != nullptr)
    {
        return func(context, count, strings, lengths, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}

cl_int clReleaseKernel(cl_kernel kernel)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clReleaseKernel_ptr;
    if (func != nullptr)
    {
        return func(kernel);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetDeviceIDs(cl_platform_id platform,
                      cl_device_type device_type,
                      cl_uint        num_entries,
                      cl_device_id  *devices,
                      cl_uint       *num_devices)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetDeviceIDs_ptr;
    if (func != nullptr)
    {
        return func(platform, device_type, num_entries, devices, num_devices);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetDeviceInfo(cl_device_id   device,
                       cl_device_info param_name,
                       size_t         param_value_size,
                       void          *param_value,
                       size_t        *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetDeviceInfo_ptr;
    if (func != nullptr)
    {
        return func(device, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetMemObjectInfo(
    cl_mem memobj, cl_mem_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetMemObjectInfo_ptr;
    if (func != nullptr)
    {
        return func(memobj, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clRetainEvent(cl_event event)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clRetainEvent_ptr;
    if (func != nullptr)
    {
        return func(event);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetPlatformInfo(cl_platform_id   platform,
                         cl_platform_info param_name,
                         size_t           param_value_size,
                         void            *param_value,
                         size_t          *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetPlatformInfo_ptr;
    if (func != nullptr)
    {
        return func(platform, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetPlatformIDs_ptr;
    if (func != nullptr)
    {
        return func(num_entries, platforms, num_platforms);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetKernelWorkGroupInfo(cl_kernel                 kernel,
                                cl_device_id              device,
                                cl_kernel_work_group_info param_name,
                                size_t                    param_value_size,
                                void                     *param_value,
                                size_t                   *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetKernelWorkGroupInfo_ptr;
    if (func != nullptr)
    {
        return func(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetCommandQueueInfo(cl_command_queue      command_queue,
                             cl_command_queue_info param_name,
                             size_t                param_value_size,
                             void                 *param_value,
                             size_t               *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetCommandQueueInfo_ptr;
    if (func != nullptr)
    {
        return func(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetKernelInfo(cl_kernel      kernel,
                       cl_kernel_info param_name,
                       size_t         param_value_size,
                       void          *param_value,
                       size_t        *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetKernelInfo_ptr;
    if (func != nullptr)
    {
        return func(kernel, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_int clGetEventProfilingInfo(cl_event          event,
                               cl_profiling_info param_name,
                               size_t            param_value_size,
                               void             *param_value,
                               size_t           *param_value_size_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clGetEventProfilingInfo_ptr;
    if (func != nullptr)
    {
        return func(event, param_name, param_value_size, param_value, param_value_size_ret);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

cl_mem clCreateImage(cl_context             context,
                     cl_mem_flags           flags,
                     const cl_image_format *image_format,
                     const cl_image_desc   *image_desc,
                     void                  *host_ptr,
                     cl_int                *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clCreateImage_ptr;
    if (func != nullptr)
    {
        return func(context, flags, image_format, image_desc, host_ptr, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}

cl_int
clSetKernelExecInfo(cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size, const void *param_value)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clSetKernelExecInfo_ptr;
    if (func != nullptr)
    {
        return func(kernel, param_name, param_value_size, param_value);
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

void *clGetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char *funcname)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clGetExtensionFunctionAddressForPlatform_ptr;

    if (func != nullptr)
    {
        return func(platform, funcname);
    }

    return nullptr;
}

cl_command_buffer_khr clCreateCommandBufferKHR(cl_uint                                 num_queues,
                                               const cl_command_queue                 *queues,
                                               const cl_command_buffer_properties_khr *properties,
                                               cl_int                                 *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clCreateCommandBufferKHR_ptr;

    if (func != nullptr)
    {
        return func(num_queues, queues, properties, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_INVALID_OPERATION;
        }

        return {};
    }
}

cl_int clFinalizeCommandBufferKHR(cl_command_buffer_khr command_buffer)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clFinalizeCommandBufferKHR_ptr;

    if (func != nullptr)
    {
        return func(command_buffer);
    }
    else
    {
        return CL_INVALID_OPERATION;
    }
}

cl_int clRetainCommandBufferKHR(cl_command_buffer_khr command_buffer)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clRetainCommandBufferKHR_ptr;

    if (func != nullptr)
    {
        return func(command_buffer);
    }
    else
    {
        return CL_INVALID_OPERATION;
    }
}

cl_int clReleaseCommandBufferKHR(cl_command_buffer_khr command_buffer)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clReleaseCommandBufferKHR_ptr;

    if (func != nullptr)
    {
        return func(command_buffer);
    }
    else
    {
        return CL_INVALID_OPERATION;
    }
}

cl_int clEnqueueCommandBufferKHR(cl_uint               num_queues,
                                 cl_command_queue     *queues,
                                 cl_command_buffer_khr command_buffer,
                                 cl_uint               num_events_in_wait_list,
                                 const cl_event       *event_wait_list,
                                 cl_event             *event)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clEnqueueCommandBufferKHR_ptr;

    if (func != nullptr)
    {
        return func(num_queues, queues, command_buffer, num_events_in_wait_list, event_wait_list, event);
    }
    else
    {
        return CL_INVALID_OPERATION;
    }
}

cl_int clCommandNDRangeKernelKHR(cl_command_buffer_khr                           command_buffer,
                                 cl_command_queue                                command_queue,
                                 const cl_ndrange_kernel_command_properties_khr *properties,
                                 cl_kernel                                       kernel,
                                 cl_uint                                         work_dim,
                                 const size_t                                   *global_work_offset,
                                 const size_t                                   *global_work_size,
                                 const size_t                                   *local_work_size,
                                 cl_uint                                         num_sync_points_in_wait_list,
                                 const cl_sync_point_khr                        *sync_point_wait_list,
                                 cl_sync_point_khr                              *sync_point,
                                 cl_mutable_command_khr                         *mutable_handle)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clCommandNDRangeKernelKHR_ptr;

    if (func != nullptr)
    {
        return func(command_buffer, command_queue, properties, kernel, work_dim, global_work_offset, global_work_size,
                    local_work_size, num_sync_points_in_wait_list, sync_point_wait_list, sync_point, mutable_handle);
    }
    else
    {
        return CL_INVALID_OPERATION;
    }
}

cl_int clUpdateMutableCommandsKHR(cl_command_buffer_khr             command_buffer,
                                  const cl_mutable_base_config_khr *mutable_config)
{
    arm_compute::CLSymbols::get().load_default();
    const auto func = arm_compute::CLSymbols::get().clUpdateMutableCommandsKHR_ptr;

    if (func != nullptr)
    {
        return func(command_buffer, mutable_config);
    }
    else
    {
        return CL_INVALID_OPERATION;
    }
}

cl_mem clImportMemoryARM(cl_context                      context,
                         cl_mem_flags                    flags,
                         const cl_import_properties_arm *properties,
                         void                           *memory,
                         size_t                          size,
                         cl_int                         *errcode_ret)
{
    arm_compute::CLSymbols::get().load_default();
    auto func = arm_compute::CLSymbols::get().clImportMemoryARM_ptr;
    if (func != nullptr)
    {
        return func(context, flags, properties, memory, size, errcode_ret);
    }
    else
    {
        if (errcode_ret != nullptr)
        {
            *errcode_ret = CL_OUT_OF_RESOURCES;
        }
        return nullptr;
    }
}