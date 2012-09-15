/*  barcode_onecode.vala
 *
 *  Copyright (C) 2012  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglbarcode.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */


using GLib;
using glbarcode.Constants;

namespace glbarcode
{

	/**
	 * ONE CODE (USPS Intelligent Mail) Barcode
	 */
	public class BarcodeOneCode : Barcode
	{

		/*
		 * Constants
		 */
		private const double ONECODE_BAR_WIDTH        = ( 0.02   * PTS_PER_INCH );
		private const double ONECODE_FULL_HEIGHT      = ( 0.145  * PTS_PER_INCH );
		private const double ONECODE_ASCENDER_HEIGHT  = ( 0.0965 * PTS_PER_INCH );
		private const double ONECODE_DESCENDER_HEIGHT = ( 0.0965 * PTS_PER_INCH );
		private const double ONECODE_TRACKER_HEIGHT   = ( 0.048  * PTS_PER_INCH );
		private const double ONECODE_FULL_OFFSET      = 0;
		private const double ONECODE_ASCENDER_OFFSET  = 0;
		private const double ONECODE_DESCENDER_OFFSET = ( 0.0485 * PTS_PER_INCH );
		private const double ONECODE_TRACKER_OFFSET   = ( 0.0485 * PTS_PER_INCH );
		private const double ONECODE_BAR_PITCH        = ( 0.0458 * PTS_PER_INCH );
		private const double ONECODE_HORIZ_MARGIN     = ( 0.125  * PTS_PER_INCH );
		private const double ONECODE_VERT_MARGIN      = ( 0.028  * PTS_PER_INCH );


		/**
		 * 104-bit Integer Representation
		 */
		private struct Int104
		{
			public uint8[] byte;

			public Int104()
			{
				byte = new uint8[13];
			}

			public void mult_uint( uint y )
			{
				uint64 carry = 0;
				for ( int i = 12; i >= 0; i-- )
				{
					uint64 temp = byte[i]*y + carry;

					byte[i] = (uint8)(temp & 0xFF);
					carry   = temp >> 8;
				}
			}

			public void add_uint64( uint64 y )
			{
				uint64 carry = 0;
				for ( int i = 12; i >= 0; i-- )
				{
					uint64 temp = byte[i] + (y&0xFF) + carry;

					byte[i] = (uint8)(temp & 0xFF);
					carry   = temp >> 8;
					y       = y >> 8;
				}
			}

			public uint div_uint( uint y )
			{
				uint carry = 0;
				for ( int i = 0; i < 13; i++ )
				{
					uint temp = byte[i] + (carry << 8);

					byte[i] = (uint8)(temp / y);
					carry   = temp % y;
				}
				return carry;
			}
		}


		/*
		 * Encoding tables
		 */
		private enum Char
		{
			A = 0,
			B = 1,
			C = 2,
			D = 3,
			E = 4,
			F = 5,
			G = 6,
			H = 7,
			I = 8,
			J = 9
		}

		private struct Bar { Char i; int mask; }

		private struct BarMapEntry
		{
			Bar descender;
			Bar ascender;
		}

		private const BarMapEntry[] bar_map = {
			/*  1 */ { { Char.H, 1<<2  }, { Char.E, 1<<3  } },
			/*  2 */ { { Char.B, 1<<10 }, { Char.A, 1<<0  } },
			/*  3 */ { { Char.J, 1<<12 }, { Char.C, 1<<8  } },
			/*  4 */ { { Char.F, 1<<5  }, { Char.G, 1<<11 } },
			/*  5 */ { { Char.I, 1<<9  }, { Char.D, 1<<1  } },
			/*  6 */ { { Char.A, 1<<1  }, { Char.F, 1<<12 } },
			/*  7 */ { { Char.C, 1<<5  }, { Char.B, 1<<8  } },
			/*  8 */ { { Char.E, 1<<4  }, { Char.J, 1<<11 } },
			/*  9 */ { { Char.G, 1<<3  }, { Char.I, 1<<10 } },
			/* 10 */ { { Char.D, 1<<9  }, { Char.H, 1<<6  } },
			/* 11 */ { { Char.F, 1<<11 }, { Char.B, 1<<4  } },
			/* 12 */ { { Char.I, 1<<5  }, { Char.C, 1<<12 } },
			/* 13 */ { { Char.J, 1<<10 }, { Char.A, 1<<2  } },
			/* 14 */ { { Char.H, 1<<1  }, { Char.G, 1<<7  } },
			/* 15 */ { { Char.D, 1<<6  }, { Char.E, 1<<9  } },
			/* 16 */ { { Char.A, 1<<3  }, { Char.I, 1<<6  } },
			/* 17 */ { { Char.G, 1<<4  }, { Char.C, 1<<7  } },
			/* 18 */ { { Char.B, 1<<1  }, { Char.J, 1<<9  } },
			/* 19 */ { { Char.H, 1<<10 }, { Char.F, 1<<2  } },
			/* 20 */ { { Char.E, 1<<0  }, { Char.D, 1<<8  } },
			/* 21 */ { { Char.G, 1<<2  }, { Char.A, 1<<4  } },
			/* 22 */ { { Char.I, 1<<11 }, { Char.B, 1<<0  } },
			/* 23 */ { { Char.J, 1<<8  }, { Char.D, 1<<12 } },
			/* 24 */ { { Char.C, 1<<6  }, { Char.H, 1<<7  } },
			/* 25 */ { { Char.F, 1<<1  }, { Char.E, 1<<10 } },
			/* 26 */ { { Char.B, 1<<12 }, { Char.G, 1<<9  } },
			/* 27 */ { { Char.H, 1<<3  }, { Char.I, 1<<0  } },
			/* 28 */ { { Char.F, 1<<8  }, { Char.J, 1<<7  } },
			/* 29 */ { { Char.E, 1<<6  }, { Char.C, 1<<10 } },
			/* 30 */ { { Char.D, 1<<4  }, { Char.A, 1<<5  } },
			/* 31 */ { { Char.I, 1<<4  }, { Char.F, 1<<7  } },
			/* 32 */ { { Char.H, 1<<11 }, { Char.B, 1<<9  } },
			/* 33 */ { { Char.G, 1<<0  }, { Char.J, 1<<6  } },
			/* 34 */ { { Char.A, 1<<6  }, { Char.E, 1<<8  } },
			/* 35 */ { { Char.C, 1<<1  }, { Char.D, 1<<2  } },
			/* 36 */ { { Char.F, 1<<9  }, { Char.I, 1<<12 } },
			/* 37 */ { { Char.E, 1<<11 }, { Char.G, 1<<1  } },
			/* 38 */ { { Char.J, 1<<5  }, { Char.H, 1<<4  } },
			/* 39 */ { { Char.D, 1<<3  }, { Char.B, 1<<2  } },
			/* 40 */ { { Char.A, 1<<7  }, { Char.C, 1<<0  } },
			/* 41 */ { { Char.B, 1<<3  }, { Char.E, 1<<1  } },
			/* 42 */ { { Char.G, 1<<10 }, { Char.D, 1<<5  } },
			/* 43 */ { { Char.I, 1<<7  }, { Char.J, 1<<4  } },
			/* 44 */ { { Char.C, 1<<11 }, { Char.F, 1<<6  } },
			/* 45 */ { { Char.A, 1<<8  }, { Char.H, 1<<12 } },
			/* 46 */ { { Char.E, 1<<2  }, { Char.I, 1<<1  } },
			/* 47 */ { { Char.F, 1<<10 }, { Char.D, 1<<0  } },
			/* 48 */ { { Char.J, 1<<3  }, { Char.A, 1<<9  } },
			/* 49 */ { { Char.G, 1<<5  }, { Char.C, 1<<4  } },
			/* 50 */ { { Char.H, 1<<8  }, { Char.B, 1<<7  } },
			/* 51 */ { { Char.F, 1<<0  }, { Char.E, 1<<5  } },
			/* 52 */ { { Char.C, 1<<3  }, { Char.A, 1<<10 } },
			/* 53 */ { { Char.G, 1<<12 }, { Char.J, 1<<2  } },
			/* 54 */ { { Char.D, 1<<11 }, { Char.B, 1<<6  } },
			/* 55 */ { { Char.I, 1<<8  }, { Char.H, 1<<9  } },
			/* 56 */ { { Char.F, 1<<4  }, { Char.A, 1<<11 } },
			/* 57 */ { { Char.B, 1<<5  }, { Char.C, 1<<2  } },
			/* 58 */ { { Char.J, 1<<1  }, { Char.E, 1<<12 } },
			/* 59 */ { { Char.I, 1<<3  }, { Char.G, 1<<6  } },
			/* 60 */ { { Char.H, 1<<0  }, { Char.D, 1<<7  } },
			/* 61 */ { { Char.E, 1<<7  }, { Char.H, 1<<5  } },
			/* 62 */ { { Char.A, 1<<12 }, { Char.B, 1<<11 } },
			/* 63 */ { { Char.C, 1<<9  }, { Char.J, 1<<0  } },
			/* 64 */ { { Char.G, 1<<8  }, { Char.F, 1<<3  } },
			/* 65 */ { { Char.D, 1<<10 }, { Char.I, 1<<2  } }
		};


		private const string[] tdaf_table = { "T", "D", "A", "F" };


		private const uint character_table[] = {
			/* Table I 5 of 13. */
			  31, 7936,   47, 7808,   55, 7552,   59, 7040,   61, 6016,
			  62, 3968,   79, 7744,   87, 7488,   91, 6976,   93, 5952,
			  94, 3904,  103, 7360,  107, 6848,  109, 5824,  110, 3776,
			 115, 6592,  117, 5568,  118, 3520,  121, 5056,  122, 3008,
			 124, 1984,  143, 7712,  151, 7456,  155, 6944,  157, 5920,
			 158, 3872,  167, 7328,  171, 6816,  173, 5792,  174, 3744,
			 179, 6560,  181, 5536,  182, 3488,  185, 5024,  186, 2976,
			 188, 1952,  199, 7264,  203, 6752,  205, 5728,  206, 3680,
			 211, 6496,  213, 5472,  214, 3424,  217, 4960,  218, 2912,
			 220, 1888,  227, 6368,  229, 5344,  230, 3296,  233, 4832,
			 234, 2784,  236, 1760,  241, 4576,  242, 2528,  244, 1504,
			 248,  992,  271, 7696,  279, 7440,  283, 6928,  285, 5904,
			 286, 3856,  295, 7312,  299, 6800,  301, 5776,  302, 3728,
			 307, 6544,  309, 5520,  310, 3472,  313, 5008,  314, 2960,
			 316, 1936,  327, 7248,  331, 6736,  333, 5712,  334, 3664,
			 339, 6480,  341, 5456,  342, 3408,  345, 4944,  346, 2896,
			 348, 1872,  355, 6352,  357, 5328,  358, 3280,  361, 4816,
			 362, 2768,  364, 1744,  369, 4560,  370, 2512,  372, 1488,
			 376,  976,  391, 7216,  395, 6704,  397, 5680,  398, 3632,
			 403, 6448,  405, 5424,  406, 3376,  409, 4912,  410, 2864,
			 412, 1840,  419, 6320,  421, 5296,  422, 3248,  425, 4784,
			 426, 2736,  428, 1712,  433, 4528,  434, 2480,  436, 1456,
			 440,  944,  451, 6256,  453, 5232,  454, 3184,  457, 4720,
			 458, 2672,  460, 1648,  465, 4464,  466, 2416,  468, 1392,
			 472,  880,  481, 4336,  482, 2288,  484, 1264,  488,  752,
			 527, 7688,  535, 7432,  539, 6920,  541, 5896,  542, 3848,
			 551, 7304,  555, 6792,  557, 5768,  558, 3720,  563, 6536,
			 565, 5512,  566, 3464,  569, 5000,  570, 2952,  572, 1928,
			 583, 7240,  587, 6728,  589, 5704,  590, 3656,  595, 6472,
			 597, 5448,  598, 3400,  601, 4936,  602, 2888,  604, 1864,
			 611, 6344,  613, 5320,  614, 3272,  617, 4808,  618, 2760,
			 620, 1736,  625, 4552,  626, 2504,  628, 1480,  632,  968,
			 647, 7208,  651, 6696,  653, 5672,  654, 3624,  659, 6440,
			 661, 5416,  662, 3368,  665, 4904,  666, 2856,  668, 1832,
			 675, 6312,  677, 5288,  678, 3240,  681, 4776,  682, 2728,
			 684, 1704,  689, 4520,  690, 2472,  692, 1448,  696,  936,
			 707, 6248,  709, 5224,  710, 3176,  713, 4712,  714, 2664,
			 716, 1640,  721, 4456,  722, 2408,  724, 1384,  728,  872,
			 737, 4328,  738, 2280,  740, 1256,  775, 7192,  779, 6680,
			 781, 5656,  782, 3608,  787, 6424,  789, 5400,  790, 3352,
			 793, 4888,  794, 2840,  796, 1816,  803, 6296,  805, 5272,
			 806, 3224,  809, 4760,  810, 2712,  812, 1688,  817, 4504,
			 818, 2456,  820, 1432,  824,  920,  835, 6232,  837, 5208,
			 838, 3160,  841, 4696,  842, 2648,  844, 1624,  849, 4440,
			 850, 2392,  852, 1368,  865, 4312,  866, 2264,  868, 1240,
			 899, 6200,  901, 5176,  902, 3128,  905, 4664,  906, 2616,
			 908, 1592,  913, 4408,  914, 2360,  916, 1336,  929, 4280,
			 930, 2232,  932, 1208,  961, 4216,  962, 2168,  964, 1144,
			1039, 7684, 1047, 7428, 1051, 6916, 1053, 5892, 1054, 3844,
			1063, 7300, 1067, 6788, 1069, 5764, 1070, 3716, 1075, 6532,
			1077, 5508, 1078, 3460, 1081, 4996, 1082, 2948, 1084, 1924,
			1095, 7236, 1099, 6724, 1101, 5700, 1102, 3652, 1107, 6468,
			1109, 5444, 1110, 3396, 1113, 4932, 1114, 2884, 1116, 1860,
			1123, 6340, 1125, 5316, 1126, 3268, 1129, 4804, 1130, 2756,
			1132, 1732, 1137, 4548, 1138, 2500, 1140, 1476, 1159, 7204,
			1163, 6692, 1165, 5668, 1166, 3620, 1171, 6436, 1173, 5412,
			1174, 3364, 1177, 4900, 1178, 2852, 1180, 1828, 1187, 6308,
			1189, 5284, 1190, 3236, 1193, 4772, 1194, 2724, 1196, 1700,
			1201, 4516, 1202, 2468, 1204, 1444, 1219, 6244, 1221, 5220,
			1222, 3172, 1225, 4708, 1226, 2660, 1228, 1636, 1233, 4452,
			1234, 2404, 1236, 1380, 1249, 4324, 1250, 2276, 1287, 7188,
			1291, 6676, 1293, 5652, 1294, 3604, 1299, 6420, 1301, 5396,
			1302, 3348, 1305, 4884, 1306, 2836, 1308, 1812, 1315, 6292,
			1317, 5268, 1318, 3220, 1321, 4756, 1322, 2708, 1324, 1684,
			1329, 4500, 1330, 2452, 1332, 1428, 1347, 6228, 1349, 5204,
			1350, 3156, 1353, 4692, 1354, 2644, 1356, 1620, 1361, 4436,
			1362, 2388, 1377, 4308, 1378, 2260, 1411, 6196, 1413, 5172,
			1414, 3124, 1417, 4660, 1418, 2612, 1420, 1588, 1425, 4404,
			1426, 2356, 1441, 4276, 1442, 2228, 1473, 4212, 1474, 2164,
			1543, 7180, 1547, 6668, 1549, 5644, 1550, 3596, 1555, 6412,
			1557, 5388, 1558, 3340, 1561, 4876, 1562, 2828, 1564, 1804,
			1571, 6284, 1573, 5260, 1574, 3212, 1577, 4748, 1578, 2700,
			1580, 1676, 1585, 4492, 1586, 2444, 1603, 6220, 1605, 5196,
			1606, 3148, 1609, 4684, 1610, 2636, 1617, 4428, 1618, 2380,
			1633, 4300, 1634, 2252, 1667, 6188, 1669, 5164, 1670, 3116,
			1673, 4652, 1674, 2604, 1681, 4396, 1682, 2348, 1697, 4268,
			1698, 2220, 1729, 4204, 1730, 2156, 1795, 6172, 1797, 5148,
			1798, 3100, 1801, 4636, 1802, 2588, 1809, 4380, 1810, 2332,
			1825, 4252, 1826, 2204, 1857, 4188, 1858, 2140, 1921, 4156,
			1922, 2108, 2063, 7682, 2071, 7426, 2075, 6914, 2077, 5890,
			2078, 3842, 2087, 7298, 2091, 6786, 2093, 5762, 2094, 3714,
			2099, 6530, 2101, 5506, 2102, 3458, 2105, 4994, 2106, 2946,
			2119, 7234, 2123, 6722, 2125, 5698, 2126, 3650, 2131, 6466,
			2133, 5442, 2134, 3394, 2137, 4930, 2138, 2882, 2147, 6338,
			2149, 5314, 2150, 3266, 2153, 4802, 2154, 2754, 2161, 4546,
			2162, 2498, 2183, 7202, 2187, 6690, 2189, 5666, 2190, 3618,
			2195, 6434, 2197, 5410, 2198, 3362, 2201, 4898, 2202, 2850,
			2211, 6306, 2213, 5282, 2214, 3234, 2217, 4770, 2218, 2722,
			2225, 4514, 2226, 2466, 2243, 6242, 2245, 5218, 2246, 3170,
			2249, 4706, 2250, 2658, 2257, 4450, 2258, 2402, 2273, 4322,
			2311, 7186, 2315, 6674, 2317, 5650, 2318, 3602, 2323, 6418,
			2325, 5394, 2326, 3346, 2329, 4882, 2330, 2834, 2339, 6290,
			2341, 5266, 2342, 3218, 2345, 4754, 2346, 2706, 2353, 4498,
			2354, 2450, 2371, 6226, 2373, 5202, 2374, 3154, 2377, 4690,
			2378, 2642, 2385, 4434, 2401, 4306, 2435, 6194, 2437, 5170,
			2438, 3122, 2441, 4658, 2442, 2610, 2449, 4402, 2465, 4274,
			2497, 4210, 2567, 7178, 2571, 6666, 2573, 5642, 2574, 3594,
			2579, 6410, 2581, 5386, 2582, 3338, 2585, 4874, 2586, 2826,
			2595, 6282, 2597, 5258, 2598, 3210, 2601, 4746, 2602, 2698,
			2609, 4490, 2627, 6218, 2629, 5194, 2630, 3146, 2633, 4682,
			2641, 4426, 2657, 4298, 2691, 6186, 2693, 5162, 2694, 3114,
			2697, 4650, 2705, 4394, 2721, 4266, 2753, 4202, 2819, 6170,
			2821, 5146, 2822, 3098, 2825, 4634, 2833, 4378, 2849, 4250,
			2881, 4186, 2945, 4154, 3079, 7174, 3083, 6662, 3085, 5638,
			3086, 3590, 3091, 6406, 3093, 5382, 3094, 3334, 3097, 4870,
			3107, 6278, 3109, 5254, 3110, 3206, 3113, 4742, 3121, 4486,
			3139, 6214, 3141, 5190, 3145, 4678, 3153, 4422, 3169, 4294,
			3203, 6182, 3205, 5158, 3209, 4646, 3217, 4390, 3233, 4262,
			3265, 4198, 3331, 6166, 3333, 5142, 3337, 4630, 3345, 4374,
			3361, 4246, 3393, 4182, 3457, 4150, 3587, 6158, 3589, 5134,
			3593, 4622, 3601, 4366, 3617, 4238, 3649, 4174, 3713, 4142,
			3841, 4126, 4111, 7681, 4119, 7425, 4123, 6913, 4125, 5889,
			4135, 7297, 4139, 6785, 4141, 5761, 4147, 6529, 4149, 5505,
			4153, 4993, 4167, 7233, 4171, 6721, 4173, 5697, 4179, 6465,
			4181, 5441, 4185, 4929, 4195, 6337, 4197, 5313, 4201, 4801,
			4209, 4545, 4231, 7201, 4235, 6689, 4237, 5665, 4243, 6433,
			4245, 5409, 4249, 4897, 4259, 6305, 4261, 5281, 4265, 4769,
			4273, 4513, 4291, 6241, 4293, 5217, 4297, 4705, 4305, 4449,
			4359, 7185, 4363, 6673, 4365, 5649, 4371, 6417, 4373, 5393,
			4377, 4881, 4387, 6289, 4389, 5265, 4393, 4753, 4401, 4497,
			4419, 6225, 4421, 5201, 4425, 4689, 4483, 6193, 4485, 5169,
			4489, 4657, 4615, 7177, 4619, 6665, 4621, 5641, 4627, 6409,
			4629, 5385, 4633, 4873, 4643, 6281, 4645, 5257, 4649, 4745,
			4675, 6217, 4677, 5193, 4739, 6185, 4741, 5161, 4867, 6169,
			4869, 5145, 5127, 7173, 5131, 6661, 5133, 5637, 5139, 6405,
			5141, 5381, 5155, 6277, 5157, 5253, 5187, 6213, 5251, 6181,
			5379, 6165, 5635, 6157, 6151, 7171, 6155, 6659, 6163, 6403,
			6179, 6275, 6211, 5189, 4681, 4433, 4321, 3142, 2634, 2386,
			2274, 1612, 1364, 1252,  856,  744,  496,
			/* Table II 2 of 13. */
			   3, 6144,    5, 5120,    6, 3072,    9, 4608,   10, 2560,
			  12, 1536,   17, 4352,   18, 2304,   20, 1280,   24,  768,
			  33, 4224,   34, 2176,   36, 1152,   40,  640,   48,  384,
			  65, 4160,   66, 2112,   68, 1088,   72,  576,   80,  320,
			  96,  192,  129, 4128,  130, 2080,  132, 1056,  136,  544,
			 144,  288,  257, 4112,  258, 2064,  260, 1040,  264,  528,
			 513, 4104,  514, 2056,  516, 1032, 1025, 4100, 1026, 2052,
			2049, 4098, 4097, 2050, 1028,  520,  272,  160
		};


		/**
		 * ONE CODE data validation method.
		 */
		protected override bool validate( string data )
		{
			if ( (data.length != 20) &&
			     (data.length != 25) &&
			     (data.length != 29) &&
			     (data.length != 31) )
			{
				return false;
			}

			for ( int i = 0; i < data.length; i++ )
			{
				if ( !data[i].isdigit() )
				{
					return false;
				}
			}

			if (data[1] > '4')
			{
				return false; /* Invalid Barcode Identifier. */
			}

			return true;
		}


		/**
		 * ONE CODE data encoding method.
		 */
		protected override string encode( string data )
		{
			Int104 value = Int104();

			/*-----------------------------------------------------------*/
			/* Step 1 -- Conversion of Data Fields into Binary Data      */
			/*-----------------------------------------------------------*/

			/* Step 1.a -- Routing Code */
			int j;
			for ( j = 20; data[j] != 0; j++ )
			{
				value.mult_uint( 10 );
				value.add_uint64( data[j] - '0' );
			}
			switch ( j-20 )
			{
			case 0:
				break;
			case 5:
				value.add_uint64( 1 );
				break;
			case 9:
				value.add_uint64( 1 );
				value.add_uint64( 100000 );
				break;
			case 11:
				value.add_uint64( 1 );
				value.add_uint64( 100000 );
				value.add_uint64( 1000000000 );
				break;
			default:
				assert_not_reached();
			}

			/* Step 1.b -- Tracking Code */
			value.mult_uint( 10 );
			value.add_uint64( data[0] - '0' );
			value.mult_uint( 5 );
			value.add_uint64( data[1] - '0' );

			for ( int i = 2; i < 20; i++ )
			{
				value.mult_uint( 10 );
				value.add_uint64( data[i] - '0' );
			}


			/*-----------------------------------------------------------*/
			/* Step 2 -- Generation of 11-Bit CRC on Binary Data         */
			/*-----------------------------------------------------------*/

			uint crc11 = USPS_MSB_Math_CRC11GenerateFrameCheckSequence( value.byte );


			/*-----------------------------------------------------------*/
			/* Step 3 -- Conversion of Binary Data to Codewords          */
			/*-----------------------------------------------------------*/
			uint[] codeword = new uint[10];

			codeword[9] = value.div_uint( 636 );
			for ( int i = 8; i >= 1; i-- )
			{
				codeword[i] = value.div_uint( 1365 );
			}
			codeword[0] = value.div_uint( 659 );


			/*-----------------------------------------------------------*/
			/* Step 4 -- Inserting Additional Information into Codewords */
			/*-----------------------------------------------------------*/

			codeword[9] *= 2;
			codeword[0] += ((crc11 & 0x400) != 0) ? 659 : 0;


			/*-----------------------------------------------------------*/
			/* Step 5 -- Conversion from Codewords to Characters         */
			/*-----------------------------------------------------------*/
			uint[] character = new uint[10];

			for ( int i = 0; i < 10; i++ )
			{
				character[i] = character_table[ codeword[i] ];

				if ( (crc11 & (1<<i)) != 0 )
				{
					character[i] = ~character[i] & 0x1FFF;
				}
			}


			/*-----------------------------------------------------------*/
			/* Step 6 -- Conversion from Characters to IMail Barcode     */
			/*-----------------------------------------------------------*/
			StringBuilder code = new StringBuilder();

			for ( int i = 0; i < 65; i++ )
			{
				int d = (character[ bar_map[i].descender.i ] & bar_map[i].descender.mask) != 0 ? 1 : 0;
				int a = (character[ bar_map[i].ascender.i ]  & bar_map[i].ascender.mask)  != 0 ? 1 : 0;

				code.append( tdaf_table[ (a<<1) + d ] );
			}


			return code.str;
		}


		/**
		 * ONE CODE vectorization method.
		 */
		protected override void vectorize( string coded_data, string data, string text )
		{
			double x = ONECODE_HORIZ_MARGIN;
			for ( int i = 0; i < coded_data.length; i++ )
			{
				double y = ONECODE_VERT_MARGIN;
				double length;

				switch ( coded_data[i] )
				{
				case 'T':
					y      += ONECODE_TRACKER_OFFSET;
					length  = ONECODE_TRACKER_HEIGHT;
					break;
				case 'D':
					y      += ONECODE_DESCENDER_OFFSET;
					length  = ONECODE_DESCENDER_HEIGHT;
					break;
				case 'A':
					y      += ONECODE_ASCENDER_OFFSET;
					length  = ONECODE_ASCENDER_HEIGHT;
					break;
				case 'F':
					y      += ONECODE_FULL_OFFSET;
					length  = ONECODE_FULL_HEIGHT;
					break;
				default:
					assert_not_reached();
				}
				double width = ONECODE_BAR_WIDTH;

				add_box( x, y, width, length );

				x += ONECODE_BAR_PITCH;
			}

			/* Overwrite requested size with actual size. */
			w = x + ONECODE_HORIZ_MARGIN;
			h = ONECODE_FULL_HEIGHT + 2 * ONECODE_VERT_MARGIN;
		}


		/***************************************************************************
		 ** USPS_MSB_Math_CRC11GenerateFrameCheckSequence
		 **
		 ** Inputs:
		 **   ByteAttayPtr is the address of a 13 byte array holding 102 bytes which
		 **   are right justified - ie: the leftmost 2 bits of the first byte do not
		 **   hold data and must be set to zero.
		 **
		 ** Outputs:
		 **   return unsigned short - 11 bit Frame Check Sequence (right justified)
		 **
		 ** From Appendix C of USPS publication USPS-B-3200E, 07/08/05.
		 ***************************************************************************/
		private uint USPS_MSB_Math_CRC11GenerateFrameCheckSequence( uint8* ByteArrayPtr )
		{
			uint  GeneratorPolynomial = 0x0F35;
			uint  FrameCheckSequence  = 0x07FF;
			uint  Data;
			int   ByteIndex, Bit;

			/* Do most significant byte skipping the 2 most significant bits */
			Data = *ByteArrayPtr << 5;
			ByteArrayPtr++;
			for ( Bit = 2; Bit < 8; Bit++ )
			{
				if ( ((FrameCheckSequence ^ Data) & 0x400) != 0 )
				{
					FrameCheckSequence = (FrameCheckSequence << 1) ^ GeneratorPolynomial;
				}
				else
				{
					FrameCheckSequence = (FrameCheckSequence << 1);
				}
				FrameCheckSequence &= 0x7FF;
				Data <<= 1;
			}

			/* Do rest of the bytes */
			for ( ByteIndex = 1; ByteIndex < 13; ByteIndex++ )
			{
				Data = *ByteArrayPtr << 3;
				ByteArrayPtr++;
				for ( Bit = 0; Bit < 8; Bit++ )
				{
					if ( ((FrameCheckSequence ^ Data) & 0x0400) != 0 )
					{
						FrameCheckSequence = (FrameCheckSequence << 1) ^ GeneratorPolynomial;
					}
					else
					{
						FrameCheckSequence = (FrameCheckSequence << 1);
					}
					FrameCheckSequence &= 0x7FF;
					Data <<= 1;
				}
			}

			return FrameCheckSequence;
		}


	}

}
