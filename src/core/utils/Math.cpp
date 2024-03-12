/*
 * Copyright (c) 2023 Arm Limited.
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

#include "src/core/utils/Math.h"

namespace arm_compute
{

const std::array<ErfLutEntry<float>, 513> erf_f32_lut = {{
    {0.0000000000f, 1.1283791671f}, // 0.0000000000
    {0.0088152829f, 1.1283102984f}, // 0.0078125000
    {0.0176294898f, 1.1281037175f}, // 0.0156250000
    {0.0264415450f, 1.1277595001f}, // 0.0234375000
    {0.0352503739f, 1.1272777722f}, // 0.0312500000
    {0.0440549026f, 1.1266587101f}, // 0.0390625000
    {0.0528540592f, 1.1259025402f}, // 0.0468750000
    {0.0616467734f, 1.1250095393f}, // 0.0546875000
    {0.0704319777f, 1.1239800336f}, // 0.0625000000
    {0.0792086070f, 1.1228143994f}, // 0.0703125000
    {0.0879755993f, 1.1215130622f}, // 0.0781250000
    {0.0967318964f, 1.1200764968f}, // 0.0859375000
    {0.1054764438f, 1.1185052270f}, // 0.0937500000
    {0.1142081913f, 1.1167998249f}, // 0.1015625000
    {0.1229260934f, 1.1149609111f}, // 0.1093750000
    {0.1316291096f, 1.1129891541f}, // 0.1171875000
    {0.1403162048f, 1.1108852696f}, // 0.1250000000
    {0.1489863498f, 1.1086500206f}, // 0.1328125000
    {0.1576385214f, 1.1062842165f}, // 0.1406250000
    {0.1662717029f, 1.1037887128f}, // 0.1484375000
    {0.1748848846f, 1.1011644107f}, // 0.1562500000
    {0.1834770639f, 1.0984122563f}, // 0.1640625000
    {0.1920472457f, 1.0955332401f}, // 0.1718750000
    {0.2005944431f, 1.0925283966f}, // 0.1796875000
    {0.2091176771f, 1.0893988035f}, // 0.1875000000
    {0.2176159774f, 1.0861455810f}, // 0.1953125000
    {0.2260883828f, 1.0827698913f}, // 0.2031250000
    {0.2345339412f, 1.0792729378f}, // 0.2109375000
    {0.2429517099f, 1.0756559646f}, // 0.2187500000
    {0.2513407564f, 1.0719202554f}, // 0.2265625000
    {0.2597001582f, 1.0680671328f}, // 0.2343750000
    {0.2680290031f, 1.0640979580f}, // 0.2421875000
    {0.2763263902f, 1.0600141294f}, // 0.2500000000
    {0.2845914291f, 1.0558170820f}, // 0.2578125000
    {0.2928232411f, 1.0515082867f}, // 0.2656250000
    {0.3010209590f, 1.0470892494f}, // 0.2734375000
    {0.3091837275f, 1.0425615098f}, // 0.2812500000
    {0.3173107036f, 1.0379266409f}, // 0.2890625000
    {0.3254010565f, 1.0331862480f}, // 0.2968750000
    {0.3334539681f, 1.0283419676f}, // 0.3046875000
    {0.3414686335f, 1.0233954666f}, // 0.3125000000
    {0.3494442605f, 1.0183484415f}, // 0.3203125000
    {0.3573800706f, 1.0132026170f}, // 0.3281250000
    {0.3652752987f, 1.0079597454f}, // 0.3359375000
    {0.3731291935f, 1.0026216055f}, // 0.3437500000
    {0.3809410179f, 0.9971900017f}, // 0.3515625000
    {0.3887100487f, 0.9916667625f}, // 0.3593750000
    {0.3964355772f, 0.9860537403f}, // 0.3671875000
    {0.4041169094f, 0.9803528095f}, // 0.3750000000
    {0.4117533659f, 0.9745658663f}, // 0.3828125000
    {0.4193442821f, 0.9686948267f}, // 0.3906250000
    {0.4268890086f, 0.9627416265f}, // 0.3984375000
    {0.4343869111f, 0.9567082195f}, // 0.4062500000
    {0.4418373708f, 0.9505965764f}, // 0.4140625000
    {0.4492397841f, 0.9444086845f}, // 0.4218750000
    {0.4565935631f, 0.9381465458f}, // 0.4296875000
    {0.4638981357f, 0.9318121761f}, // 0.4375000000
    {0.4711529456f, 0.9254076045f}, // 0.4453125000
    {0.4783574521f, 0.9189348715f}, // 0.4531250000
    {0.4855111308f, 0.9123960286f}, // 0.4609375000
    {0.4926134732f, 0.9057931368f}, // 0.4687500000
    {0.4996639871f, 0.8991282656f}, // 0.4765625000
    {0.5066621964f, 0.8924034924f}, // 0.4843750000
    {0.5136076411f, 0.8856209005f}, // 0.4921875000
    {0.5204998778f, 0.8787825789f}, // 0.5000000000
    {0.5273384792f, 0.8718906210f}, // 0.5078125000
    {0.5341230345f, 0.8649471234f}, // 0.5156250000
    {0.5408531493f, 0.8579541846f}, // 0.5234375000
    {0.5475284454f, 0.8509139049f}, // 0.5312500000
    {0.5541485612f, 0.8438283842f}, // 0.5390625000
    {0.5607131516f, 0.8366997220f}, // 0.5468750000
    {0.5672218875f, 0.8295300154f}, // 0.5546875000
    {0.5736744566f, 0.8223213592f}, // 0.5625000000
    {0.5800705628f, 0.8150758439f}, // 0.5703125000
    {0.5864099261f, 0.8077955554f}, // 0.5781250000
    {0.5926922832f, 0.8004825735f}, // 0.5859375000
    {0.5989173866f, 0.7931389715f}, // 0.5937500000
    {0.6050850052f, 0.7857668149f}, // 0.6015625000
    {0.6111949241f, 0.7783681603f}, // 0.6093750000
    {0.6172469441f, 0.7709450550f}, // 0.6171875000
    {0.6232408822f, 0.7634995358f}, // 0.6250000000
    {0.6291765712f, 0.7560336278f}, // 0.6328125000
    {0.6350538598f, 0.7485493443f}, // 0.6406250000
    {0.6408726121f, 0.7410486852f}, // 0.6484375000
    {0.6466327080f, 0.7335336365f}, // 0.6562500000
    {0.6523340428f, 0.7260061695f}, // 0.6640625000
    {0.6579765272f, 0.7184682397f}, // 0.6718750000
    {0.6635600869f, 0.7109217866f}, // 0.6796875000
    {0.6690846629f, 0.7033687322f}, // 0.6875000000
    {0.6745502111f, 0.6958109807f}, // 0.6953125000
    {0.6799567021f, 0.6882504177f}, // 0.7031250000
    {0.6853041214f, 0.6806889096f}, // 0.7109375000
    {0.6905924687f, 0.6731283025f}, // 0.7187500000
    {0.6958217582f, 0.6655704219f}, // 0.7265625000
    {0.7009920183f, 0.6580170718f}, // 0.7343750000
    {0.7061032914f, 0.6504700344f}, // 0.7421875000
    {0.7111556337f, 0.6429310692f}, // 0.7500000000
    {0.7161491149f, 0.6354019123f}, // 0.7578125000
    {0.7210838185f, 0.6278842762f}, // 0.7656250000
    {0.7259598411f, 0.6203798491f}, // 0.7734375000
    {0.7307772924f, 0.6128902940f}, // 0.7812500000
    {0.7355362950f, 0.6054172488f}, // 0.7890625000
    {0.7402369841f, 0.5979623254f}, // 0.7968750000
    {0.7448795076f, 0.5905271095f}, // 0.8046875000
    {0.7494640256f, 0.5831131598f}, // 0.8125000000
    {0.7539907101f, 0.5757220079f}, // 0.8203125000
    {0.7584597452f, 0.5683551577f}, // 0.8281250000
    {0.7628713266f, 0.5610140853f}, // 0.8359375000
    {0.7672256612f, 0.5537002383f}, // 0.8437500000
    {0.7715229674f, 0.5464150355f}, // 0.8515625000
    {0.7757634744f, 0.5391598669f}, // 0.8593750000
    {0.7799474221f, 0.5319360931f}, // 0.8671875000
    {0.7840750611f, 0.5247450453f}, // 0.8750000000
    {0.7881466520f, 0.5175880246f}, // 0.8828125000
    {0.7921624659f, 0.5104663022f}, // 0.8906250000
    {0.7961227832f, 0.5033811191f}, // 0.8984375000
    {0.8000278942f, 0.4963336858f}, // 0.9062500000
    {0.8038780984f, 0.4893251822f}, // 0.9140625000
    {0.8076737045f, 0.4823567575f}, // 0.9218750000
    {0.8114150300f, 0.4754295299f}, // 0.9296875000
    {0.8151024010f, 0.4685445869f}, // 0.9375000000
    {0.8187361521f, 0.4617029846f}, // 0.9453125000
    {0.8223166257f, 0.4549057483f}, // 0.9531250000
    {0.8258441725f, 0.4481538720f}, // 0.9609375000
    {0.8293191506f, 0.4414483184f}, // 0.9687500000
    {0.8327419255f, 0.4347900193f}, // 0.9765625000
    {0.8361128701f, 0.4281798750f}, // 0.9843750000
    {0.8394323638f, 0.4216187550f}, // 0.9921875000
    {0.8427007929f, 0.4151074974f}, // 1.0000000000
    {0.8459185504f, 0.4086469096f}, // 1.0078125000
    {0.8490860349f, 0.4022377678f}, // 1.0156250000
    {0.8522036514f, 0.3958808176f}, // 1.0234375000
    {0.8552718104f, 0.3895767737f}, // 1.0312500000
    {0.8582909280f, 0.3833263203f}, // 1.0390625000
    {0.8612614255f, 0.3771301114f}, // 1.0468750000
    {0.8641837289f, 0.3709887705f}, // 1.0546875000
    {0.8670582694f, 0.3649028912f}, // 1.0625000000
    {0.8698854825f, 0.3588730371f}, // 1.0703125000
    {0.8726658079f, 0.3528997425f}, // 1.0781250000
    {0.8753996896f, 0.3469835119f}, // 1.0859375000
    {0.8780875752f, 0.3411248209f}, // 1.0937500000
    {0.8807299159f, 0.3353241162f}, // 1.1015625000
    {0.8833271666f, 0.3295818158f}, // 1.1093750000
    {0.8858797849f, 0.3238983093f}, // 1.1171875000
    {0.8883882317f, 0.3182739585f}, // 1.1250000000
    {0.8908529704f, 0.3127090972f}, // 1.1328125000
    {0.8932744671f, 0.3072040319f}, // 1.1406250000
    {0.8956531899f, 0.3017590421f}, // 1.1484375000
    {0.8979896092f, 0.2963743805f}, // 1.1562500000
    {0.9002841973f, 0.2910502733f}, // 1.1640625000
    {0.9025374279f, 0.2857869208f}, // 1.1718750000
    {0.9047497766f, 0.2805844976f}, // 1.1796875000
    {0.9069217198f, 0.2754431531f}, // 1.1875000000
    {0.9090537352f, 0.2703630118f}, // 1.1953125000
    {0.9111463015f, 0.2653441734f}, // 1.2031250000
    {0.9131998978f, 0.2603867140f}, // 1.2109375000
    {0.9152150039f, 0.2554906858f}, // 1.2187500000
    {0.9171920998f, 0.2506561176f}, // 1.2265625000
    {0.9191316658f, 0.2458830155f}, // 1.2343750000
    {0.9210341819f, 0.2411713632f}, // 1.2421875000
    {0.9229001283f, 0.2365211224f}, // 1.2500000000
    {0.9247299843f, 0.2319322334f}, // 1.2578125000
    {0.9265242290f, 0.2274046151f}, // 1.2656250000
    {0.9282833407f, 0.2229381659f}, // 1.2734375000
    {0.9300077968f, 0.2185327643f}, // 1.2812500000
    {0.9316980737f, 0.2141882685f}, // 1.2890625000
    {0.9333546467f, 0.2099045180f}, // 1.2968750000
    {0.9349779895f, 0.2056813330f}, // 1.3046875000
    {0.9365685747f, 0.2015185157f}, // 1.3125000000
    {0.9381268730f, 0.1974158503f}, // 1.3203125000
    {0.9396533534f, 0.1933731034f}, // 1.3281250000
    {0.9411484831f, 0.1893900249f}, // 1.3359375000
    {0.9426127272f, 0.1854663482f}, // 1.3437500000
    {0.9440465488f, 0.1816017904f}, // 1.3515625000
    {0.9454504084f, 0.1777960534f}, // 1.3593750000
    {0.9468247645f, 0.1740488238f}, // 1.3671875000
    {0.9481700728f, 0.1703597737f}, // 1.3750000000
    {0.9494867865f, 0.1667285609f}, // 1.3828125000
    {0.9507753562f, 0.1631548298f}, // 1.3906250000
    {0.9520362295f, 0.1596382112f}, // 1.3984375000
    {0.9532698510f, 0.1561783236f}, // 1.4062500000
    {0.9544766625f, 0.1527747727f}, // 1.4140625000
    {0.9556571025f, 0.1494271527f}, // 1.4218750000
    {0.9568116063f, 0.1461350463f}, // 1.4296875000
    {0.9579406061f, 0.1428980254f}, // 1.4375000000
    {0.9590445303f, 0.1397156511f}, // 1.4453125000
    {0.9601238042f, 0.1365874749f}, // 1.4531250000
    {0.9611788495f, 0.1335130382f}, // 1.4609375000
    {0.9622100842f, 0.1304918737f}, // 1.4687500000
    {0.9632179226f, 0.1275235050f}, // 1.4765625000
    {0.9642027752f, 0.1246074475f}, // 1.4843750000
    {0.9651650489f, 0.1217432089f}, // 1.4921875000
    {0.9661051465f, 0.1189302892f}, // 1.5000000000
    {0.9670234670f, 0.1161681815f}, // 1.5078125000
    {0.9679204053f, 0.1134563721f}, // 1.5156250000
    {0.9687963524f, 0.1107943411f}, // 1.5234375000
    {0.9696516951f, 0.1081815630f}, // 1.5312500000
    {0.9704868162f, 0.1056175064f}, // 1.5390625000
    {0.9713020942f, 0.1031016352f}, // 1.5468750000
    {0.9720979033f, 0.1006334084f}, // 1.5546875000
    {0.9728746138f, 0.0982122808f}, // 1.5625000000
    {0.9736325914f, 0.0958377032f}, // 1.5703125000
    {0.9743721977f, 0.0935091227f}, // 1.5781250000
    {0.9750937898f, 0.0912259834f}, // 1.5859375000
    {0.9757977206f, 0.0889877264f}, // 1.5937500000
    {0.9764843385f, 0.0867937900f}, // 1.6015625000
    {0.9771539875f, 0.0846436106f}, // 1.6093750000
    {0.9778070074f, 0.0825366227f}, // 1.6171875000
    {0.9784437332f, 0.0804722590f}, // 1.6250000000
    {0.9790644959f, 0.0784499511f}, // 1.6328125000
    {0.9796696218f, 0.0764691297f}, // 1.6406250000
    {0.9802594326f, 0.0745292246f}, // 1.6484375000
    {0.9808342460f, 0.0726296655f}, // 1.6562500000
    {0.9813943747f, 0.0707698819f}, // 1.6640625000
    {0.9819401275f, 0.0689493034f}, // 1.6718750000
    {0.9824718082f, 0.0671673602f}, // 1.6796875000
    {0.9829897166f, 0.0654234833f}, // 1.6875000000
    {0.9834941478f, 0.0637171046f}, // 1.6953125000
    {0.9839853925f, 0.0620476570f}, // 1.7031250000
    {0.9844637371f, 0.0604145752f}, // 1.7109375000
    {0.9849294635f, 0.0588172956f}, // 1.7187500000
    {0.9853828492f, 0.0572552562f}, // 1.7265625000
    {0.9858241672f, 0.0557278976f}, // 1.7343750000
    {0.9862536864f, 0.0542346624f}, // 1.7421875000
    {0.9866716712f, 0.0527749959f}, // 1.7500000000
    {0.9870783817f, 0.0513483463f}, // 1.7578125000
    {0.9874740737f, 0.0499541645f}, // 1.7656250000
    {0.9878589987f, 0.0485919049f}, // 1.7734375000
    {0.9882334039f, 0.0472610247f}, // 1.7812500000
    {0.9885975325f, 0.0459609852f}, // 1.7890625000
    {0.9889516232f, 0.0446912508f}, // 1.7968750000
    {0.9892959108f, 0.0434512901f}, // 1.8046875000
    {0.9896306258f, 0.0422405756f}, // 1.8125000000
    {0.9899559946f, 0.0410585838f}, // 1.8203125000
    {0.9902722396f, 0.0399047954f}, // 1.8281250000
    {0.9905795791f, 0.0387786956f}, // 1.8359375000
    {0.9908782275f, 0.0376797741f}, // 1.8437500000
    {0.9911683951f, 0.0366075252f}, // 1.8515625000
    {0.9914502882f, 0.0355614479f}, // 1.8593750000
    {0.9917241096f, 0.0345410460f}, // 1.8671875000
    {0.9919900577f, 0.0335458284f}, // 1.8750000000
    {0.9922483274f, 0.0325753089f}, // 1.8828125000
    {0.9924991099f, 0.0316290065f}, // 1.8906250000
    {0.9927425925f, 0.0307064452f}, // 1.8984375000
    {0.9929789587f, 0.0298071547f}, // 1.9062500000
    {0.9932083887f, 0.0289306696f}, // 1.9140625000
    {0.9934310586f, 0.0280765301f}, // 1.9218750000
    {0.9936471415f, 0.0272442821f}, // 1.9296875000
    {0.9938568064f, 0.0264334768f}, // 1.9375000000
    {0.9940602192f, 0.0256436709f}, // 1.9453125000
    {0.9942575423f, 0.0248744271f}, // 1.9531250000
    {0.9944489346f, 0.0241253134f}, // 1.9609375000
    {0.9946345516f, 0.0233959038f}, // 1.9687500000
    {0.9948145458f, 0.0226857778f}, // 1.9765625000
    {0.9949890661f, 0.0219945210f}, // 1.9843750000
    {0.9951582582f, 0.0213217245f}, // 1.9921875000
    {0.9953222650f, 0.0206669854f}, // 2.0000000000
    {0.9954812259f, 0.0200299065f}, // 2.0078125000
    {0.9956352773f, 0.0194100966f}, // 2.0156250000
    {0.9957845526f, 0.0188071704f}, // 2.0234375000
    {0.9959291823f, 0.0182207482f}, // 2.0312500000
    {0.9960692938f, 0.0176504563f}, // 2.0390625000
    {0.9962050117f, 0.0170959271f}, // 2.0468750000
    {0.9963364578f, 0.0165567984f}, // 2.0546875000
    {0.9964637509f, 0.0160327141f}, // 2.0625000000
    {0.9965870072f, 0.0155233240f}, // 2.0703125000
    {0.9967063402f, 0.0150282836f}, // 2.0781250000
    {0.9968218606f, 0.0145472542f}, // 2.0859375000
    {0.9969336766f, 0.0140799029f}, // 2.0937500000
    {0.9970418939f, 0.0136259025f}, // 2.1015625000
    {0.9971466153f, 0.0131849315f}, // 2.1093750000
    {0.9972479415f, 0.0127566743f}, // 2.1171875000
    {0.9973459706f, 0.0123408206f}, // 2.1250000000
    {0.9974407984f, 0.0119370661f}, // 2.1328125000
    {0.9975325180f, 0.0115451118f}, // 2.1406250000
    {0.9976212207f, 0.0111646644f}, // 2.1484375000
    {0.9977069951f, 0.0107954360f}, // 2.1562500000
    {0.9977899279f, 0.0104371443f}, // 2.1640625000
    {0.9978701033f, 0.0100895123f}, // 2.1718750000
    {0.9979476035f, 0.0097522684f}, // 2.1796875000
    {0.9980225088f, 0.0094251464f}, // 2.1875000000
    {0.9980948971f, 0.0091078852f}, // 2.1953125000
    {0.9981648445f, 0.0088002291f}, // 2.2031250000
    {0.9982324251f, 0.0085019274f}, // 2.2109375000
    {0.9982977109f, 0.0082127346f}, // 2.2187500000
    {0.9983607721f, 0.0079324104f}, // 2.2265625000
    {0.9984216773f, 0.0076607192f}, // 2.2343750000
    {0.9984804928f, 0.0073974307f}, // 2.2421875000
    {0.9985372834f, 0.0071423190f}, // 2.2500000000
    {0.9985921122f, 0.0068951636f}, // 2.2578125000
    {0.9986450405f, 0.0066557482f}, // 2.2656250000
    {0.9986961279f, 0.0064238617f}, // 2.2734375000
    {0.9987454324f, 0.0061992973f}, // 2.2812500000
    {0.9987930105f, 0.0059818530f}, // 2.2890625000
    {0.9988389169f, 0.0057713311f}, // 2.2968750000
    {0.9988832050f, 0.0055675385f}, // 2.3046875000
    {0.9989259267f, 0.0053702865f}, // 2.3125000000
    {0.9989671323f, 0.0051793907f}, // 2.3203125000
    {0.9990068708f, 0.0049946708f}, // 2.3281250000
    {0.9990451897f, 0.0048159509f}, // 2.3359375000
    {0.9990821352f, 0.0046430592f}, // 2.3437500000
    {0.9991177522f, 0.0044758278f}, // 2.3515625000
    {0.9991520843f, 0.0043140931f}, // 2.3593750000
    {0.9991851738f, 0.0041576951f}, // 2.3671875000
    {0.9992170618f, 0.0040064779f}, // 2.3750000000
    {0.9992477881f, 0.0038602892f}, // 2.3828125000
    {0.9992773915f, 0.0037189807f}, // 2.3906250000
    {0.9993059095f, 0.0035824076f}, // 2.3984375000
    {0.9993333786f, 0.0034504286f}, // 2.4062500000
    {0.9993598341f, 0.0033229062f}, // 2.4140625000
    {0.9993853103f, 0.0031997062f}, // 2.4218750000
    {0.9994098404f, 0.0030806979f}, // 2.4296875000
    {0.9994334567f, 0.0029657539f}, // 2.4375000000
    {0.9994561906f, 0.0028547501f}, // 2.4453125000
    {0.9994780722f, 0.0027475655f}, // 2.4531250000
    {0.9994991309f, 0.0026440825f}, // 2.4609375000
    {0.9995193953f, 0.0025441865f}, // 2.4687500000
    {0.9995388929f, 0.0024477658f}, // 2.4765625000
    {0.9995576504f, 0.0023547119f}, // 2.4843750000
    {0.9995756937f, 0.0022649190f}, // 2.4921875000
    {0.9995930480f, 0.0021782842f}, // 2.5000000000
    {0.9996097374f, 0.0020947076f}, // 2.5078125000
    {0.9996257855f, 0.0020140918f}, // 2.5156250000
    {0.9996412150f, 0.0019363421f}, // 2.5234375000
    {0.9996560481f, 0.0018613666f}, // 2.5312500000
    {0.9996703059f, 0.0017890757f}, // 2.5390625000
    {0.9996840091f, 0.0017193826f}, // 2.5468750000
    {0.9996971778f, 0.0016522026f}, // 2.5546875000
    {0.9997098311f, 0.0015874537f}, // 2.5625000000
    {0.9997219879f, 0.0015250561f}, // 2.5703125000
    {0.9997336661f, 0.0014649323f}, // 2.5781250000
    {0.9997448832f, 0.0014070070f}, // 2.5859375000
    {0.9997556561f, 0.0013512073f}, // 2.5937500000
    {0.9997660011f, 0.0012974620f}, // 2.6015625000
    {0.9997759341f, 0.0012457025f}, // 2.6093750000
    {0.9997854702f, 0.0011958618f}, // 2.6171875000
    {0.9997946243f, 0.0011478751f}, // 2.6250000000
    {0.9998034104f, 0.0011016795f}, // 2.6328125000
    {0.9998118425f, 0.0010572140f}, // 2.6406250000
    {0.9998199338f, 0.0010144193f}, // 2.6484375000
    {0.9998276970f, 0.0009732381f}, // 2.6562500000
    {0.9998351447f, 0.0009336147f}, // 2.6640625000
    {0.9998422887f, 0.0008954951f}, // 2.6718750000
    {0.9998491406f, 0.0008588272f}, // 2.6796875000
    {0.9998557115f, 0.0008235601f}, // 2.6875000000
    {0.9998620122f, 0.0007896449f}, // 2.6953125000
    {0.9998680531f, 0.0007570339f}, // 2.7031250000
    {0.9998738441f, 0.0007256811f}, // 2.7109375000
    {0.9998793950f, 0.0006955419f}, // 2.7187500000
    {0.9998847150f, 0.0006665730f}, // 2.7265625000
    {0.9998898132f, 0.0006387328f}, // 2.7343750000
    {0.9998946981f, 0.0006119806f}, // 2.7421875000
    {0.9998993781f, 0.0005862772f}, // 2.7500000000
    {0.9999038613f, 0.0005615849f}, // 2.7578125000
    {0.9999081554f, 0.0005378669f}, // 2.7656250000
    {0.9999122679f, 0.0005150877f}, // 2.7734375000
    {0.9999162060f, 0.0004932131f}, // 2.7812500000
    {0.9999199766f, 0.0004722097f}, // 2.7890625000
    {0.9999235864f, 0.0004520456f}, // 2.7968750000
    {0.9999270419f, 0.0004326897f}, // 2.8046875000
    {0.9999303492f, 0.0004141120f}, // 2.8125000000
    {0.9999335144f, 0.0003962836f}, // 2.8203125000
    {0.9999365431f, 0.0003791765f}, // 2.8281250000
    {0.9999394408f, 0.0003627636f}, // 2.8359375000
    {0.9999422130f, 0.0003470187f}, // 2.8437500000
    {0.9999448647f, 0.0003319167f}, // 2.8515625000
    {0.9999474008f, 0.0003174332f}, // 2.8593750000
    {0.9999498261f, 0.0003035446f}, // 2.8671875000
    {0.9999521452f, 0.0002902283f}, // 2.8750000000
    {0.9999543624f, 0.0002774622f}, // 2.8828125000
    {0.9999564819f, 0.0002652254f}, // 2.8906250000
    {0.9999585078f, 0.0002534972f}, // 2.8984375000
    {0.9999604441f, 0.0002422581f}, // 2.9062500000
    {0.9999622943f, 0.0002314890f}, // 2.9140625000
    {0.9999640622f, 0.0002211717f}, // 2.9218750000
    {0.9999657513f, 0.0002112884f}, // 2.9296875000
    {0.9999673647f, 0.0002018221f}, // 2.9375000000
    {0.9999689058f, 0.0001927564f}, // 2.9453125000
    {0.9999703775f, 0.0001840754f}, // 2.9531250000
    {0.9999717829f, 0.0001757640f}, // 2.9609375000
    {0.9999731248f, 0.0001678073f}, // 2.9687500000
    {0.9999744058f, 0.0001601913f}, // 2.9765625000
    {0.9999756286f, 0.0001529022f}, // 2.9843750000
    {0.9999767957f, 0.0001459270f}, // 2.9921875000
    {0.9999779095f, 0.0001392531f}, // 3.0000000000
    {0.9999789723f, 0.0001328681f}, // 3.0078125000
    {0.9999799863f, 0.0001267604f}, // 3.0156250000
    {0.9999809536f, 0.0001209187f}, // 3.0234375000
    {0.9999818763f, 0.0001153322f}, // 3.0312500000
    {0.9999827563f, 0.0001099903f}, // 3.0390625000
    {0.9999835955f, 0.0001048830f}, // 3.0468750000
    {0.9999843957f, 0.0001000007f}, // 3.0546875000
    {0.9999851586f, 0.0000953340f}, // 3.0625000000
    {0.9999858858f, 0.0000908740f}, // 3.0703125000
    {0.9999865790f, 0.0000866121f}, // 3.0781250000
    {0.9999872396f, 0.0000825400f}, // 3.0859375000
    {0.9999878692f, 0.0000786497f}, // 3.0937500000
    {0.9999884690f, 0.0000749336f}, // 3.1015625000
    {0.9999890404f, 0.0000713844f}, // 3.1093750000
    {0.9999895848f, 0.0000679950f}, // 3.1171875000
    {0.9999901033f, 0.0000647587f}, // 3.1250000000
    {0.9999905970f, 0.0000616688f}, // 3.1328125000
    {0.9999910672f, 0.0000587192f}, // 3.1406250000
    {0.9999915149f, 0.0000559039f}, // 3.1484375000
    {0.9999919410f, 0.0000532170f}, // 3.1562500000
    {0.9999923467f, 0.0000506531f}, // 3.1640625000
    {0.9999927328f, 0.0000482069f}, // 3.1718750000
    {0.9999931002f, 0.0000458732f}, // 3.1796875000
    {0.9999934498f, 0.0000436471f}, // 3.1875000000
    {0.9999937825f, 0.0000415240f}, // 3.1953125000
    {0.9999940989f, 0.0000394993f}, // 3.2031250000
    {0.9999943999f, 0.0000375688f}, // 3.2109375000
    {0.9999946862f, 0.0000357282f}, // 3.2187500000
    {0.9999949584f, 0.0000339737f}, // 3.2265625000
    {0.9999952172f, 0.0000323014f}, // 3.2343750000
    {0.9999954633f, 0.0000307077f}, // 3.2421875000
    {0.9999956972f, 0.0000291890f}, // 3.2500000000
    {0.9999959196f, 0.0000277421f}, // 3.2578125000
    {0.9999961309f, 0.0000263636f}, // 3.2656250000
    {0.9999963317f, 0.0000250506f}, // 3.2734375000
    {0.9999965224f, 0.0000238001f}, // 3.2812500000
    {0.9999967037f, 0.0000226093f}, // 3.2890625000
    {0.9999968759f, 0.0000214754f}, // 3.2968750000
    {0.9999970394f, 0.0000203959f}, // 3.3046875000
    {0.9999971947f, 0.0000193683f}, // 3.3125000000
    {0.9999973421f, 0.0000183902f}, // 3.3203125000
    {0.9999974822f, 0.0000174594f}, // 3.3281250000
    {0.9999976151f, 0.0000165736f}, // 3.3359375000
    {0.9999977412f, 0.0000157309f}, // 3.3437500000
    {0.9999978610f, 0.0000149292f}, // 3.3515625000
    {0.9999979746f, 0.0000141667f}, // 3.3593750000
    {0.9999980824f, 0.0000134414f}, // 3.3671875000
    {0.9999981847f, 0.0000127517f}, // 3.3750000000
    {0.9999982818f, 0.0000120960f}, // 3.3828125000
    {0.9999983738f, 0.0000114725f}, // 3.3906250000
    {0.9999984611f, 0.0000108799f}, // 3.3984375000
    {0.9999985439f, 0.0000103166f}, // 3.4062500000
    {0.9999986224f, 0.0000097813f}, // 3.4140625000
    {0.9999986968f, 0.0000092726f}, // 3.4218750000
    {0.9999987673f, 0.0000087893f}, // 3.4296875000
    {0.9999988342f, 0.0000083302f}, // 3.4375000000
    {0.9999988975f, 0.0000078941f}, // 3.4453125000
    {0.9999989576f, 0.0000074799f}, // 3.4531250000
    {0.9999990145f, 0.0000070866f}, // 3.4609375000
    {0.9999990684f, 0.0000067131f}, // 3.4687500000
    {0.9999991194f, 0.0000063586f}, // 3.4765625000
    {0.9999991678f, 0.0000060220f}, // 3.4843750000
    {0.9999992135f, 0.0000057026f}, // 3.4921875000
    {0.9999992569f, 0.0000053994f}, // 3.5000000000
    {0.9999992980f, 0.0000051118f}, // 3.5078125000
    {0.9999993368f, 0.0000048388f}, // 3.5156250000
    {0.9999993736f, 0.0000045799f}, // 3.5234375000
    {0.9999994084f, 0.0000043343f}, // 3.5312500000
    {0.9999994414f, 0.0000041014f}, // 3.5390625000
    {0.9999994725f, 0.0000038805f}, // 3.5468750000
    {0.9999995020f, 0.0000036711f}, // 3.5546875000
    {0.9999995299f, 0.0000034725f}, // 3.5625000000
    {0.9999995563f, 0.0000032843f}, // 3.5703125000
    {0.9999995813f, 0.0000031059f}, // 3.5781250000
    {0.9999996049f, 0.0000029369f}, // 3.5859375000
    {0.9999996272f, 0.0000027767f}, // 3.5937500000
    {0.9999996483f, 0.0000026249f}, // 3.6015625000
    {0.9999996682f, 0.0000024811f}, // 3.6093750000
    {0.9999996870f, 0.0000023449f}, // 3.6171875000
    {0.9999997049f, 0.0000022159f}, // 3.6250000000
    {0.9999997217f, 0.0000020938f}, // 3.6328125000
    {0.9999997376f, 0.0000019781f}, // 3.6406250000
    {0.9999997526f, 0.0000018686f}, // 3.6484375000
    {0.9999997668f, 0.0000017650f}, // 3.6562500000
    {0.9999997802f, 0.0000016669f}, // 3.6640625000
    {0.9999997929f, 0.0000015740f}, // 3.6718750000
    {0.9999998048f, 0.0000014862f}, // 3.6796875000
    {0.9999998161f, 0.0000014030f}, // 3.6875000000
    {0.9999998267f, 0.0000013244f}, // 3.6953125000
    {0.9999998368f, 0.0000012500f}, // 3.7031250000
    {0.9999998463f, 0.0000011797f}, // 3.7109375000
    {0.9999998552f, 0.0000011131f}, // 3.7187500000
    {0.9999998637f, 0.0000010502f}, // 3.7265625000
    {0.9999998717f, 0.0000009908f}, // 3.7343750000
    {0.9999998792f, 0.0000009346f}, // 3.7421875000
    {0.9999998863f, 0.0000008814f}, // 3.7500000000
    {0.9999998930f, 0.0000008312f}, // 3.7578125000
    {0.9999998993f, 0.0000007838f}, // 3.7656250000
    {0.9999999052f, 0.0000007389f}, // 3.7734375000
    {0.9999999108f, 0.0000006966f}, // 3.7812500000
    {0.9999999161f, 0.0000006566f}, // 3.7890625000
    {0.9999999211f, 0.0000006188f}, // 3.7968750000
    {0.9999999258f, 0.0000005831f}, // 3.8046875000
    {0.9999999302f, 0.0000005494f}, // 3.8125000000
    {0.9999999344f, 0.0000005176f}, // 3.8203125000
    {0.9999999383f, 0.0000004876f}, // 3.8281250000
    {0.9999999420f, 0.0000004593f}, // 3.8359375000
    {0.9999999455f, 0.0000004325f}, // 3.8437500000
    {0.9999999488f, 0.0000004073f}, // 3.8515625000
    {0.9999999518f, 0.0000003835f}, // 3.8593750000
    {0.9999999547f, 0.0000003610f}, // 3.8671875000
    {0.9999999575f, 0.0000003398f}, // 3.8750000000
    {0.9999999601f, 0.0000003198f}, // 3.8828125000
    {0.9999999625f, 0.0000003010f}, // 3.8906250000
    {0.9999999648f, 0.0000002832f}, // 3.8984375000
    {0.9999999669f, 0.0000002665f}, // 3.9062500000
    {0.9999999689f, 0.0000002507f}, // 3.9140625000
    {0.9999999708f, 0.0000002358f}, // 3.9218750000
    {0.9999999726f, 0.0000002218f}, // 3.9296875000
    {0.9999999743f, 0.0000002085f}, // 3.9375000000
    {0.9999999759f, 0.0000001961f}, // 3.9453125000
    {0.9999999774f, 0.0000001844f}, // 3.9531250000
    {0.9999999788f, 0.0000001733f}, // 3.9609375000
    {0.9999999801f, 0.0000001629f}, // 3.9687500000
    {0.9999999813f, 0.0000001531f}, // 3.9765625000
    {0.9999999825f, 0.0000001439f}, // 3.9843750000
    {0.9999999836f, 0.0000001352f}, // 3.9921875000
    {0.9999999846f, 0.0000001270f}, // 4.0000000000
}};

} // namespace arm_compute
