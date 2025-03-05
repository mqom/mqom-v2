#if defined(RIJNDAEL_TABLE)
#include "rijndael_table.h"

/* Useful macros */
/*
 * 32-bit integer manipulation macros (little endian)
 */
#ifndef GET_UINT32_LE
#define GET_UINT32_LE(n,b,i)                            \
do {                                                    \
    (n) = ( (uint32_t) (b)[(i)    ]       )             \
	| ( (uint32_t) (b)[(i) + 1] <<	8 )		\
	| ( (uint32_t) (b)[(i) + 2] << 16 )		\
	| ( (uint32_t) (b)[(i) + 3] << 24 );		\
} while( 0 )
#endif

#ifndef PUT_UINT32_LE
#define PUT_UINT32_LE(n,b,i)                                    \
do {                                                            \
    (b)[(i)    ] = (uint8_t) ( ( (n)       ) & 0xFF );    \
    (b)[(i) + 1] = (uint8_t) ( ( (n) >>  8 ) & 0xFF );    \
    (b)[(i) + 2] = (uint8_t) ( ( (n) >> 16 ) & 0xFF );    \
    (b)[(i) + 3] = (uint8_t) ( ( (n) >> 24 ) & 0xFF );    \
} while( 0 )
#endif

/* Tabulated AES (use T-tables to be more efficient) */

/* S-Box */
static const uint8_t sbox[256] = {
     0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
     0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
     0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
     0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
     0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
     0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
     0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
     0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
     0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
     0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
     0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
     0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
     0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
     0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
     0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
     0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

/* Encryption tables */

static const uint32_t Te0[256] = {
	0xa56363c6, 0x847c7cf8, 0x997777ee, 0x8d7b7bf6,
	0x0df2f2ff, 0xbd6b6bd6, 0xb16f6fde, 0x54c5c591,
	0x50303060, 0x03010102, 0xa96767ce, 0x7d2b2b56,
	0x19fefee7, 0x62d7d7b5, 0xe6abab4d, 0x9a7676ec,
	0x45caca8f, 0x9d82821f, 0x40c9c989, 0x877d7dfa,
	0x15fafaef, 0xeb5959b2, 0xc947478e, 0x0bf0f0fb,
	0xecadad41, 0x67d4d4b3, 0xfda2a25f, 0xeaafaf45,
	0xbf9c9c23, 0xf7a4a453, 0x967272e4, 0x5bc0c09b,
	0xc2b7b775, 0x1cfdfde1, 0xae93933d, 0x6a26264c,
	0x5a36366c, 0x413f3f7e, 0x02f7f7f5, 0x4fcccc83,
	0x5c343468, 0xf4a5a551, 0x34e5e5d1, 0x08f1f1f9,
	0x937171e2, 0x73d8d8ab, 0x53313162, 0x3f15152a,
	0x0c040408, 0x52c7c795, 0x65232346, 0x5ec3c39d,
	0x28181830, 0xa1969637, 0x0f05050a, 0xb59a9a2f,
	0x0907070e, 0x36121224, 0x9b80801b, 0x3de2e2df,
	0x26ebebcd, 0x6927274e, 0xcdb2b27f, 0x9f7575ea,
	0x1b090912, 0x9e83831d, 0x742c2c58, 0x2e1a1a34,
	0x2d1b1b36, 0xb26e6edc, 0xee5a5ab4, 0xfba0a05b,
	0xf65252a4, 0x4d3b3b76, 0x61d6d6b7, 0xceb3b37d,
	0x7b292952, 0x3ee3e3dd, 0x712f2f5e, 0x97848413,
	0xf55353a6, 0x68d1d1b9, 0x00000000, 0x2cededc1,
	0x60202040, 0x1ffcfce3, 0xc8b1b179, 0xed5b5bb6,
	0xbe6a6ad4, 0x46cbcb8d, 0xd9bebe67, 0x4b393972,
	0xde4a4a94, 0xd44c4c98, 0xe85858b0, 0x4acfcf85,
	0x6bd0d0bb, 0x2aefefc5, 0xe5aaaa4f, 0x16fbfbed,
	0xc5434386, 0xd74d4d9a, 0x55333366, 0x94858511,
	0xcf45458a, 0x10f9f9e9, 0x06020204, 0x817f7ffe,
	0xf05050a0, 0x443c3c78, 0xba9f9f25, 0xe3a8a84b,
	0xf35151a2, 0xfea3a35d, 0xc0404080, 0x8a8f8f05,
	0xad92923f, 0xbc9d9d21, 0x48383870, 0x04f5f5f1,
	0xdfbcbc63, 0xc1b6b677, 0x75dadaaf, 0x63212142,
	0x30101020, 0x1affffe5, 0x0ef3f3fd, 0x6dd2d2bf,
	0x4ccdcd81, 0x140c0c18, 0x35131326, 0x2fececc3,
	0xe15f5fbe, 0xa2979735, 0xcc444488, 0x3917172e,
	0x57c4c493, 0xf2a7a755, 0x827e7efc, 0x473d3d7a,
	0xac6464c8, 0xe75d5dba, 0x2b191932, 0x957373e6,
	0xa06060c0, 0x98818119, 0xd14f4f9e, 0x7fdcdca3,
	0x66222244, 0x7e2a2a54, 0xab90903b, 0x8388880b,
	0xca46468c, 0x29eeeec7, 0xd3b8b86b, 0x3c141428,
	0x79dedea7, 0xe25e5ebc, 0x1d0b0b16, 0x76dbdbad,
	0x3be0e0db, 0x56323264, 0x4e3a3a74, 0x1e0a0a14,
	0xdb494992, 0x0a06060c, 0x6c242448, 0xe45c5cb8,
	0x5dc2c29f, 0x6ed3d3bd, 0xefacac43, 0xa66262c4,
	0xa8919139, 0xa4959531, 0x37e4e4d3, 0x8b7979f2,
	0x32e7e7d5, 0x43c8c88b, 0x5937376e, 0xb76d6dda,
	0x8c8d8d01, 0x64d5d5b1, 0xd24e4e9c, 0xe0a9a949,
	0xb46c6cd8, 0xfa5656ac, 0x07f4f4f3, 0x25eaeacf,
	0xaf6565ca, 0x8e7a7af4, 0xe9aeae47, 0x18080810,
	0xd5baba6f, 0x887878f0, 0x6f25254a, 0x722e2e5c,
	0x241c1c38, 0xf1a6a657, 0xc7b4b473, 0x51c6c697,
	0x23e8e8cb, 0x7cdddda1, 0x9c7474e8, 0x211f1f3e,
	0xdd4b4b96, 0xdcbdbd61, 0x868b8b0d, 0x858a8a0f,
	0x907070e0, 0x423e3e7c, 0xc4b5b571, 0xaa6666cc,
	0xd8484890, 0x05030306, 0x01f6f6f7, 0x120e0e1c,
	0xa36161c2, 0x5f35356a, 0xf95757ae, 0xd0b9b969,
	0x91868617, 0x58c1c199, 0x271d1d3a, 0xb99e9e27,
	0x38e1e1d9, 0x13f8f8eb, 0xb398982b, 0x33111122,
	0xbb6969d2, 0x70d9d9a9, 0x898e8e07, 0xa7949433,
	0xb69b9b2d, 0x221e1e3c, 0x92878715, 0x20e9e9c9,
	0x49cece87, 0xff5555aa, 0x78282850, 0x7adfdfa5,
	0x8f8c8c03, 0xf8a1a159, 0x80898909, 0x170d0d1a,
	0xdabfbf65, 0x31e6e6d7, 0xc6424284, 0xb86868d0,
	0xc3414182, 0xb0999929, 0x772d2d5a, 0x110f0f1e,
	0xcbb0b07b, 0xfc5454a8, 0xd6bbbb6d, 0x3a16162c,
};

static const uint32_t Te1[256] = {
	0x6363c6a5, 0x7c7cf884, 0x7777ee99, 0x7b7bf68d,
	0xf2f2ff0d, 0x6b6bd6bd, 0x6f6fdeb1, 0xc5c59154,
	0x30306050, 0x01010203, 0x6767cea9, 0x2b2b567d,
	0xfefee719, 0xd7d7b562, 0xabab4de6, 0x7676ec9a,
	0xcaca8f45, 0x82821f9d, 0xc9c98940, 0x7d7dfa87,
	0xfafaef15, 0x5959b2eb, 0x47478ec9, 0xf0f0fb0b,
	0xadad41ec, 0xd4d4b367, 0xa2a25ffd, 0xafaf45ea,
	0x9c9c23bf, 0xa4a453f7, 0x7272e496, 0xc0c09b5b,
	0xb7b775c2, 0xfdfde11c, 0x93933dae, 0x26264c6a,
	0x36366c5a, 0x3f3f7e41, 0xf7f7f502, 0xcccc834f,
	0x3434685c, 0xa5a551f4, 0xe5e5d134, 0xf1f1f908,
	0x7171e293, 0xd8d8ab73, 0x31316253, 0x15152a3f,
	0x0404080c, 0xc7c79552, 0x23234665, 0xc3c39d5e,
	0x18183028, 0x969637a1, 0x05050a0f, 0x9a9a2fb5,
	0x07070e09, 0x12122436, 0x80801b9b, 0xe2e2df3d,
	0xebebcd26, 0x27274e69, 0xb2b27fcd, 0x7575ea9f,
	0x0909121b, 0x83831d9e, 0x2c2c5874, 0x1a1a342e,
	0x1b1b362d, 0x6e6edcb2, 0x5a5ab4ee, 0xa0a05bfb,
	0x5252a4f6, 0x3b3b764d, 0xd6d6b761, 0xb3b37dce,
	0x2929527b, 0xe3e3dd3e, 0x2f2f5e71, 0x84841397,
	0x5353a6f5, 0xd1d1b968, 0x00000000, 0xededc12c,
	0x20204060, 0xfcfce31f, 0xb1b179c8, 0x5b5bb6ed,
	0x6a6ad4be, 0xcbcb8d46, 0xbebe67d9, 0x3939724b,
	0x4a4a94de, 0x4c4c98d4, 0x5858b0e8, 0xcfcf854a,
	0xd0d0bb6b, 0xefefc52a, 0xaaaa4fe5, 0xfbfbed16,
	0x434386c5, 0x4d4d9ad7, 0x33336655, 0x85851194,
	0x45458acf, 0xf9f9e910, 0x02020406, 0x7f7ffe81,
	0x5050a0f0, 0x3c3c7844, 0x9f9f25ba, 0xa8a84be3,
	0x5151a2f3, 0xa3a35dfe, 0x404080c0, 0x8f8f058a,
	0x92923fad, 0x9d9d21bc, 0x38387048, 0xf5f5f104,
	0xbcbc63df, 0xb6b677c1, 0xdadaaf75, 0x21214263,
	0x10102030, 0xffffe51a, 0xf3f3fd0e, 0xd2d2bf6d,
	0xcdcd814c, 0x0c0c1814, 0x13132635, 0xececc32f,
	0x5f5fbee1, 0x979735a2, 0x444488cc, 0x17172e39,
	0xc4c49357, 0xa7a755f2, 0x7e7efc82, 0x3d3d7a47,
	0x6464c8ac, 0x5d5dbae7, 0x1919322b, 0x7373e695,
	0x6060c0a0, 0x81811998, 0x4f4f9ed1, 0xdcdca37f,
	0x22224466, 0x2a2a547e, 0x90903bab, 0x88880b83,
	0x46468cca, 0xeeeec729, 0xb8b86bd3, 0x1414283c,
	0xdedea779, 0x5e5ebce2, 0x0b0b161d, 0xdbdbad76,
	0xe0e0db3b, 0x32326456, 0x3a3a744e, 0x0a0a141e,
	0x494992db, 0x06060c0a, 0x2424486c, 0x5c5cb8e4,
	0xc2c29f5d, 0xd3d3bd6e, 0xacac43ef, 0x6262c4a6,
	0x919139a8, 0x959531a4, 0xe4e4d337, 0x7979f28b,
	0xe7e7d532, 0xc8c88b43, 0x37376e59, 0x6d6ddab7,
	0x8d8d018c, 0xd5d5b164, 0x4e4e9cd2, 0xa9a949e0,
	0x6c6cd8b4, 0x5656acfa, 0xf4f4f307, 0xeaeacf25,
	0x6565caaf, 0x7a7af48e, 0xaeae47e9, 0x08081018,
	0xbaba6fd5, 0x7878f088, 0x25254a6f, 0x2e2e5c72,
	0x1c1c3824, 0xa6a657f1, 0xb4b473c7, 0xc6c69751,
	0xe8e8cb23, 0xdddda17c, 0x7474e89c, 0x1f1f3e21,
	0x4b4b96dd, 0xbdbd61dc, 0x8b8b0d86, 0x8a8a0f85,
	0x7070e090, 0x3e3e7c42, 0xb5b571c4, 0x6666ccaa,
	0x484890d8, 0x03030605, 0xf6f6f701, 0x0e0e1c12,
	0x6161c2a3, 0x35356a5f, 0x5757aef9, 0xb9b969d0,
	0x86861791, 0xc1c19958, 0x1d1d3a27, 0x9e9e27b9,
	0xe1e1d938, 0xf8f8eb13, 0x98982bb3, 0x11112233,
	0x6969d2bb, 0xd9d9a970, 0x8e8e0789, 0x949433a7,
	0x9b9b2db6, 0x1e1e3c22, 0x87871592, 0xe9e9c920,
	0xcece8749, 0x5555aaff, 0x28285078, 0xdfdfa57a,
	0x8c8c038f, 0xa1a159f8, 0x89890980, 0x0d0d1a17,
	0xbfbf65da, 0xe6e6d731, 0x424284c6, 0x6868d0b8,
	0x414182c3, 0x999929b0, 0x2d2d5a77, 0x0f0f1e11,
	0xb0b07bcb, 0x5454a8fc, 0xbbbb6dd6, 0x16162c3a,
};

static const uint32_t Te2[256] = {
	0x63c6a563, 0x7cf8847c, 0x77ee9977, 0x7bf68d7b,
	0xf2ff0df2, 0x6bd6bd6b, 0x6fdeb16f, 0xc59154c5,
	0x30605030, 0x01020301, 0x67cea967, 0x2b567d2b,
	0xfee719fe, 0xd7b562d7, 0xab4de6ab, 0x76ec9a76,
	0xca8f45ca, 0x821f9d82, 0xc98940c9, 0x7dfa877d,
	0xfaef15fa, 0x59b2eb59, 0x478ec947, 0xf0fb0bf0,
	0xad41ecad, 0xd4b367d4, 0xa25ffda2, 0xaf45eaaf,
	0x9c23bf9c, 0xa453f7a4, 0x72e49672, 0xc09b5bc0,
	0xb775c2b7, 0xfde11cfd, 0x933dae93, 0x264c6a26,
	0x366c5a36, 0x3f7e413f, 0xf7f502f7, 0xcc834fcc,
	0x34685c34, 0xa551f4a5, 0xe5d134e5, 0xf1f908f1,
	0x71e29371, 0xd8ab73d8, 0x31625331, 0x152a3f15,
	0x04080c04, 0xc79552c7, 0x23466523, 0xc39d5ec3,
	0x18302818, 0x9637a196, 0x050a0f05, 0x9a2fb59a,
	0x070e0907, 0x12243612, 0x801b9b80, 0xe2df3de2,
	0xebcd26eb, 0x274e6927, 0xb27fcdb2, 0x75ea9f75,
	0x09121b09, 0x831d9e83, 0x2c58742c, 0x1a342e1a,
	0x1b362d1b, 0x6edcb26e, 0x5ab4ee5a, 0xa05bfba0,
	0x52a4f652, 0x3b764d3b, 0xd6b761d6, 0xb37dceb3,
	0x29527b29, 0xe3dd3ee3, 0x2f5e712f, 0x84139784,
	0x53a6f553, 0xd1b968d1, 0x00000000, 0xedc12ced,
	0x20406020, 0xfce31ffc, 0xb179c8b1, 0x5bb6ed5b,
	0x6ad4be6a, 0xcb8d46cb, 0xbe67d9be, 0x39724b39,
	0x4a94de4a, 0x4c98d44c, 0x58b0e858, 0xcf854acf,
	0xd0bb6bd0, 0xefc52aef, 0xaa4fe5aa, 0xfbed16fb,
	0x4386c543, 0x4d9ad74d, 0x33665533, 0x85119485,
	0x458acf45, 0xf9e910f9, 0x02040602, 0x7ffe817f,
	0x50a0f050, 0x3c78443c, 0x9f25ba9f, 0xa84be3a8,
	0x51a2f351, 0xa35dfea3, 0x4080c040, 0x8f058a8f,
	0x923fad92, 0x9d21bc9d, 0x38704838, 0xf5f104f5,
	0xbc63dfbc, 0xb677c1b6, 0xdaaf75da, 0x21426321,
	0x10203010, 0xffe51aff, 0xf3fd0ef3, 0xd2bf6dd2,
	0xcd814ccd, 0x0c18140c, 0x13263513, 0xecc32fec,
	0x5fbee15f, 0x9735a297, 0x4488cc44, 0x172e3917,
	0xc49357c4, 0xa755f2a7, 0x7efc827e, 0x3d7a473d,
	0x64c8ac64, 0x5dbae75d, 0x19322b19, 0x73e69573,
	0x60c0a060, 0x81199881, 0x4f9ed14f, 0xdca37fdc,
	0x22446622, 0x2a547e2a, 0x903bab90, 0x880b8388,
	0x468cca46, 0xeec729ee, 0xb86bd3b8, 0x14283c14,
	0xdea779de, 0x5ebce25e, 0x0b161d0b, 0xdbad76db,
	0xe0db3be0, 0x32645632, 0x3a744e3a, 0x0a141e0a,
	0x4992db49, 0x060c0a06, 0x24486c24, 0x5cb8e45c,
	0xc29f5dc2, 0xd3bd6ed3, 0xac43efac, 0x62c4a662,
	0x9139a891, 0x9531a495, 0xe4d337e4, 0x79f28b79,
	0xe7d532e7, 0xc88b43c8, 0x376e5937, 0x6ddab76d,
	0x8d018c8d, 0xd5b164d5, 0x4e9cd24e, 0xa949e0a9,
	0x6cd8b46c, 0x56acfa56, 0xf4f307f4, 0xeacf25ea,
	0x65caaf65, 0x7af48e7a, 0xae47e9ae, 0x08101808,
	0xba6fd5ba, 0x78f08878, 0x254a6f25, 0x2e5c722e,
	0x1c38241c, 0xa657f1a6, 0xb473c7b4, 0xc69751c6,
	0xe8cb23e8, 0xdda17cdd, 0x74e89c74, 0x1f3e211f,
	0x4b96dd4b, 0xbd61dcbd, 0x8b0d868b, 0x8a0f858a,
	0x70e09070, 0x3e7c423e, 0xb571c4b5, 0x66ccaa66,
	0x4890d848, 0x03060503, 0xf6f701f6, 0x0e1c120e,
	0x61c2a361, 0x356a5f35, 0x57aef957, 0xb969d0b9,
	0x86179186, 0xc19958c1, 0x1d3a271d, 0x9e27b99e,
	0xe1d938e1, 0xf8eb13f8, 0x982bb398, 0x11223311,
	0x69d2bb69, 0xd9a970d9, 0x8e07898e, 0x9433a794,
	0x9b2db69b, 0x1e3c221e, 0x87159287, 0xe9c920e9,
	0xce8749ce, 0x55aaff55, 0x28507828, 0xdfa57adf,
	0x8c038f8c, 0xa159f8a1, 0x89098089, 0x0d1a170d,
	0xbf65dabf, 0xe6d731e6, 0x4284c642, 0x68d0b868,
	0x4182c341, 0x9929b099, 0x2d5a772d, 0x0f1e110f,
	0xb07bcbb0, 0x54a8fc54, 0xbb6dd6bb, 0x162c3a16,
};

static const uint32_t Te3[256] = {
	0xc6a56363, 0xf8847c7c, 0xee997777, 0xf68d7b7b,
	0xff0df2f2, 0xd6bd6b6b, 0xdeb16f6f, 0x9154c5c5,
	0x60503030, 0x02030101, 0xcea96767, 0x567d2b2b,
	0xe719fefe, 0xb562d7d7, 0x4de6abab, 0xec9a7676,
	0x8f45caca, 0x1f9d8282, 0x8940c9c9, 0xfa877d7d,
	0xef15fafa, 0xb2eb5959, 0x8ec94747, 0xfb0bf0f0,
	0x41ecadad, 0xb367d4d4, 0x5ffda2a2, 0x45eaafaf,
	0x23bf9c9c, 0x53f7a4a4, 0xe4967272, 0x9b5bc0c0,
	0x75c2b7b7, 0xe11cfdfd, 0x3dae9393, 0x4c6a2626,
	0x6c5a3636, 0x7e413f3f, 0xf502f7f7, 0x834fcccc,
	0x685c3434, 0x51f4a5a5, 0xd134e5e5, 0xf908f1f1,
	0xe2937171, 0xab73d8d8, 0x62533131, 0x2a3f1515,
	0x080c0404, 0x9552c7c7, 0x46652323, 0x9d5ec3c3,
	0x30281818, 0x37a19696, 0x0a0f0505, 0x2fb59a9a,
	0x0e090707, 0x24361212, 0x1b9b8080, 0xdf3de2e2,
	0xcd26ebeb, 0x4e692727, 0x7fcdb2b2, 0xea9f7575,
	0x121b0909, 0x1d9e8383, 0x58742c2c, 0x342e1a1a,
	0x362d1b1b, 0xdcb26e6e, 0xb4ee5a5a, 0x5bfba0a0,
	0xa4f65252, 0x764d3b3b, 0xb761d6d6, 0x7dceb3b3,
	0x527b2929, 0xdd3ee3e3, 0x5e712f2f, 0x13978484,
	0xa6f55353, 0xb968d1d1, 0x00000000, 0xc12ceded,
	0x40602020, 0xe31ffcfc, 0x79c8b1b1, 0xb6ed5b5b,
	0xd4be6a6a, 0x8d46cbcb, 0x67d9bebe, 0x724b3939,
	0x94de4a4a, 0x98d44c4c, 0xb0e85858, 0x854acfcf,
	0xbb6bd0d0, 0xc52aefef, 0x4fe5aaaa, 0xed16fbfb,
	0x86c54343, 0x9ad74d4d, 0x66553333, 0x11948585,
	0x8acf4545, 0xe910f9f9, 0x04060202, 0xfe817f7f,
	0xa0f05050, 0x78443c3c, 0x25ba9f9f, 0x4be3a8a8,
	0xa2f35151, 0x5dfea3a3, 0x80c04040, 0x058a8f8f,
	0x3fad9292, 0x21bc9d9d, 0x70483838, 0xf104f5f5,
	0x63dfbcbc, 0x77c1b6b6, 0xaf75dada, 0x42632121,
	0x20301010, 0xe51affff, 0xfd0ef3f3, 0xbf6dd2d2,
	0x814ccdcd, 0x18140c0c, 0x26351313, 0xc32fecec,
	0xbee15f5f, 0x35a29797, 0x88cc4444, 0x2e391717,
	0x9357c4c4, 0x55f2a7a7, 0xfc827e7e, 0x7a473d3d,
	0xc8ac6464, 0xbae75d5d, 0x322b1919, 0xe6957373,
	0xc0a06060, 0x19988181, 0x9ed14f4f, 0xa37fdcdc,
	0x44662222, 0x547e2a2a, 0x3bab9090, 0x0b838888,
	0x8cca4646, 0xc729eeee, 0x6bd3b8b8, 0x283c1414,
	0xa779dede, 0xbce25e5e, 0x161d0b0b, 0xad76dbdb,
	0xdb3be0e0, 0x64563232, 0x744e3a3a, 0x141e0a0a,
	0x92db4949, 0x0c0a0606, 0x486c2424, 0xb8e45c5c,
	0x9f5dc2c2, 0xbd6ed3d3, 0x43efacac, 0xc4a66262,
	0x39a89191, 0x31a49595, 0xd337e4e4, 0xf28b7979,
	0xd532e7e7, 0x8b43c8c8, 0x6e593737, 0xdab76d6d,
	0x018c8d8d, 0xb164d5d5, 0x9cd24e4e, 0x49e0a9a9,
	0xd8b46c6c, 0xacfa5656, 0xf307f4f4, 0xcf25eaea,
	0xcaaf6565, 0xf48e7a7a, 0x47e9aeae, 0x10180808,
	0x6fd5baba, 0xf0887878, 0x4a6f2525, 0x5c722e2e,
	0x38241c1c, 0x57f1a6a6, 0x73c7b4b4, 0x9751c6c6,
	0xcb23e8e8, 0xa17cdddd, 0xe89c7474, 0x3e211f1f,
	0x96dd4b4b, 0x61dcbdbd, 0x0d868b8b, 0x0f858a8a,
	0xe0907070, 0x7c423e3e, 0x71c4b5b5, 0xccaa6666,
	0x90d84848, 0x06050303, 0xf701f6f6, 0x1c120e0e,
	0xc2a36161, 0x6a5f3535, 0xaef95757, 0x69d0b9b9,
	0x17918686, 0x9958c1c1, 0x3a271d1d, 0x27b99e9e,
	0xd938e1e1, 0xeb13f8f8, 0x2bb39898, 0x22331111,
	0xd2bb6969, 0xa970d9d9, 0x07898e8e, 0x33a79494,
	0x2db69b9b, 0x3c221e1e, 0x15928787, 0xc920e9e9,
	0x8749cece, 0xaaff5555, 0x50782828, 0xa57adfdf,
	0x038f8c8c, 0x59f8a1a1, 0x09808989, 0x1a170d0d,
	0x65dabfbf, 0xd731e6e6, 0x84c64242, 0xd0b86868,
	0x82c34141, 0x29b09999, 0x5a772d2d, 0x1e110f0f,
	0x7bcbb0b0, 0xa8fc5454, 0x6dd6bbbb, 0x2c3a1616,
};

/* RCON constants table */
static const uint32_t rcon[14] = {
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x0000001b, 0x00000036, 0x0000006c, 0x000000d8,
        0x000000ab, 0x0000004d
};

/* Rijndael primitives *******************************************************/
/************************************************************************/

/* Encryption key schedule */
int rijndael_setkey_enc(rijndael_table_ctx *ctx, const uint8_t *key, rijndael_type rtype)
{
	uint32_t i;
	/* Get the round keys pointer as uint32_t (NOTE: buffer should be 4 bytes aligned) */
	uint32_t *RK = (uint32_t*)(ctx->rk);
	int ret = -1;

	if((ctx == NULL) || (key == NULL)){
		goto err;
	}
	switch(rtype){
		case AES128:{
			ctx->Nr = 10;
			ctx->Nk = 4;
			ctx->Nb = 4;
			break;
		}
		case AES256:{
			ctx->Nr = 14;
			ctx->Nk = 8;
			ctx->Nb = 4;
			break;
		}
		case RIJNDAEL_256_256:{
			ctx->Nr = 14;
			ctx->Nk = 8;
			ctx->Nb = 8;
			break;
		}
		default:{
			ret = -1;
			goto err;
		}
	}
	ctx->rtype = rtype;

	/* Perform the key schedule */
	for(i = 0; i < ctx->Nk; i++){
		GET_UINT32_LE(RK[i], key, (4*i));
	}

	switch(ctx->Nr){
		case 10:{
			for(i = 0; i < 10; i++){
				RK[4]  = RK[0] ^ rcon[i] ^
					( (uint32_t) sbox[ ( RK[3] >>  8 ) & 0xFF ]       ) ^
					( (uint32_t) sbox[ ( RK[3] >> 16 ) & 0xFF ] <<  8 ) ^
					( (uint32_t) sbox[ ( RK[3] >> 24 ) & 0xFF ] << 16 ) ^
					( (uint32_t) sbox[ ( RK[3]       ) & 0xFF ] << 24 );
				RK[5]  = RK[1] ^ RK[4];
				RK[6]  = RK[2] ^ RK[5];
				RK[7]  = RK[3] ^ RK[6];
				RK += 4;
			}
			break;
		}
		case 12:{
			for(i = 0; i < 8; i++){
				RK[6]  = RK[0] ^ rcon[i] ^
					( (uint32_t) sbox[ ( RK[5] >>  8 ) & 0xFF ]       ) ^
					( (uint32_t) sbox[ ( RK[5] >> 16 ) & 0xFF ] <<  8 ) ^
					( (uint32_t) sbox[ ( RK[5] >> 24 ) & 0xFF ] << 16 ) ^
					( (uint32_t) sbox[ ( RK[5]       ) & 0xFF ] << 24 );
				RK[7]  = RK[1] ^ RK[6];
				RK[8]  = RK[2] ^ RK[7];
				RK[9]  = RK[3] ^ RK[8];
				RK[10] = RK[4] ^ RK[9];
				RK[11] = RK[5] ^ RK[10];
				RK += 6;
			}
			break;
		}
		case 14:{
			unsigned int rcon_offset = 0;
			for(i = 0; i < 7; i++){
				RK[8]  = RK[0] ^ rcon[rcon_offset++] ^
					( (uint32_t) sbox[ ( RK[7] >>  8 ) & 0xFF ]       ) ^
					( (uint32_t) sbox[ ( RK[7] >> 16 ) & 0xFF ] <<  8 ) ^
					( (uint32_t) sbox[ ( RK[7] >> 24 ) & 0xFF ] << 16 ) ^
					( (uint32_t) sbox[ ( RK[7]       ) & 0xFF ] << 24 );
				RK[9]  = RK[1] ^ RK[8];
				RK[10] = RK[2] ^ RK[9];
				RK[11] = RK[3] ^ RK[10];
				RK[12] = RK[4] ^
					( (uint32_t) sbox[ ( RK[11]       ) & 0xFF ]       ) ^
					( (uint32_t) sbox[ ( RK[11] >>  8 ) & 0xFF ] <<  8 ) ^
					( (uint32_t) sbox[ ( RK[11] >> 16 ) & 0xFF ] << 16 ) ^
					( (uint32_t) sbox[ ( RK[11] >> 24 ) & 0xFF ] << 24 );
				RK[13] = RK[5] ^ RK[12];
				RK[14] = RK[6] ^ RK[13];
				RK[15] = RK[7] ^ RK[14];
				RK += 8;
				if(ctx->Nb == 8){
					/* Rijndael with 256 block size case */
					RK[8]  = RK[0] ^ rcon[rcon_offset++] ^
						( (uint32_t) sbox[ ( RK[7] >>  8 ) & 0xFF ]       ) ^
						( (uint32_t) sbox[ ( RK[7] >> 16 ) & 0xFF ] <<  8 ) ^
						( (uint32_t) sbox[ ( RK[7] >> 24 ) & 0xFF ] << 16 ) ^
						( (uint32_t) sbox[ ( RK[7]       ) & 0xFF ] << 24 );
					RK[9]  = RK[1] ^ RK[8];
					RK[10] = RK[2] ^ RK[9];
					RK[11] = RK[3] ^ RK[10];
					RK[12] = RK[4] ^
						( (uint32_t) sbox[ ( RK[11]       ) & 0xFF ]       ) ^
						( (uint32_t) sbox[ ( RK[11] >>  8 ) & 0xFF ] <<  8 ) ^
						( (uint32_t) sbox[ ( RK[11] >> 16 ) & 0xFF ] << 16 ) ^
						( (uint32_t) sbox[ ( RK[11] >> 24 ) & 0xFF ] << 24 );
					RK[13] = RK[5] ^ RK[12];
					RK[14] = RK[6] ^ RK[13];
					RK[15] = RK[7] ^ RK[14];
					RK += 8;
				}
			}
			break;
		}
		default:{
			goto err;
		}
	}

	ret = 0;

err:
	return ret;
}

/* Encryption primitives */
/* The 128 bits block encryption */
#define RIJNDAEL_ENC_ROUND_16(X0, X1, X2, X3, Y0, Y1, Y2, Y3)	  \
do {							  \
    (X0) = (*RK) ^ Te0[ ( (Y0)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y1) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y2) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y3) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X1) = (*RK) ^ Te0[ ( (Y1)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y2) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y3) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y0) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X2) = (*RK) ^ Te0[ ( (Y2)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y3) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y0) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y1) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X3) = (*RK) ^ Te0[ ( (Y3)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y0) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y1) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y2) >> 24 ) & 0xFF ]; RK++;	  \
} while( 0 )

#define RIJNDAEL_ENC_LASTROUND_16(X0, X1, X2, X3, Y0, Y1, Y2, Y3) do {  \
	X0 = (*RK) ^							\
	    ( (uint32_t) sbox[ ( Y0	  ) & 0xFF ]	   ) ^		\
	    ( (uint32_t) sbox[ ( Y1 >>  8 ) & 0xFF ] <<  8 ) ^		\
	    ( (uint32_t) sbox[ ( Y2 >> 16 ) & 0xFF ] << 16 ) ^		\
	    ( (uint32_t) sbox[ ( Y3 >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	X1 = (*RK) ^							\
	    ( (uint32_t) sbox[ ( Y1       ) & 0xFF ]       ) ^		\
	    ( (uint32_t) sbox[ ( Y2 >>  8 ) & 0xFF ] <<  8 ) ^		\
	    ( (uint32_t) sbox[ ( Y3 >> 16 ) & 0xFF ] << 16 ) ^		\
	    ( (uint32_t) sbox[ ( Y0 >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	X2 = (*RK) ^							\
	   ( (uint32_t) sbox[ ( Y2       ) & 0xFF ]       ) ^		\
	   ( (uint32_t) sbox[ ( Y3 >>  8 ) & 0xFF ] <<  8 ) ^		\
	   ( (uint32_t) sbox[ ( Y0 >> 16 ) & 0xFF ] << 16 ) ^		\
	   ( (uint32_t) sbox[ ( Y1 >> 24 ) & 0xFF ] << 24 ); RK++; 	\
									\
	X3 = (*RK) ^							\
	   ( (uint32_t) sbox[ ( Y3       ) & 0xFF ]       ) ^		\
	   ( (uint32_t) sbox[ ( Y0 >>  8 ) & 0xFF ] <<  8 ) ^		\
	   ( (uint32_t) sbox[ ( Y1 >> 16 ) & 0xFF ] << 16 ) ^		\
	   ( (uint32_t) sbox[ ( Y2 >> 24 ) & 0xFF ] << 24 ); RK++;	\
} while(0);

/* The 256 bits encryption */
#define RIJNDAEL_ENC_ROUND_32(X0, X1, X2, X3, X4, X5, X6, X7, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7)	  \
do {							  \
    (X0) = (*RK) ^ Te0[ ( (Y0)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y1) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y3) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y4) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X1) = (*RK) ^ Te0[ ( (Y1)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y2) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y4) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y5) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X2) = (*RK) ^ Te0[ ( (Y2)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y3) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y5) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y6) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X3) = (*RK) ^ Te0[ ( (Y3)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y4) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y6) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y7) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X4) = (*RK) ^ Te0[ ( (Y4)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y5) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y7) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y0) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X5) = (*RK) ^ Te0[ ( (Y5)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y6) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y0) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y1) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X6) = (*RK) ^ Te0[ ( (Y6)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y7) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y1) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y2) >> 24 ) & 0xFF ]; RK++;	  \
							  \
    (X7) = (*RK) ^ Te0[ ( (Y7)	     ) & 0xFF ] ^	  \
		   Te1[ ( (Y0) >>  8 ) & 0xFF ] ^	  \
		   Te2[ ( (Y2) >> 16 ) & 0xFF ] ^	  \
		   Te3[ ( (Y3) >> 24 ) & 0xFF ]; RK++;	  \
							  \
} while( 0 )

#define RIJNDAEL_ENC_LASTROUND_32(X0, X1, X2, X3, X4, X5, X6, X7, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7) do {  \
	(X0) = (*RK) ^							\
	    ( (uint32_t) sbox[ ( (Y0)	  ) & 0xFF ]	   ) ^		\
	    ( (uint32_t) sbox[ ( (Y1) >>  8 ) & 0xFF ] <<  8 ) ^	\
	    ( (uint32_t) sbox[ ( (Y3) >> 16 ) & 0xFF ] << 16 ) ^	\
	    ( (uint32_t) sbox[ ( (Y4) >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	(X1) = (*RK) ^							\
	    ( (uint32_t) sbox[ ( (Y1)       ) & 0xFF ]       ) ^	\
	    ( (uint32_t) sbox[ ( (Y2) >>  8 ) & 0xFF ] <<  8 ) ^	\
	    ( (uint32_t) sbox[ ( (Y4) >> 16 ) & 0xFF ] << 16 ) ^	\
	    ( (uint32_t) sbox[ ( (Y5) >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	(X2) = (*RK) ^							\
	   ( (uint32_t) sbox[ ( (Y2)       ) & 0xFF ]       ) ^		\
	   ( (uint32_t) sbox[ ( (Y3) >>  8 ) & 0xFF ] <<  8 ) ^		\
	   ( (uint32_t) sbox[ ( (Y5) >> 16 ) & 0xFF ] << 16 ) ^		\
	   ( (uint32_t) sbox[ ( (Y6) >> 24 ) & 0xFF ] << 24 ); RK++; 	\
									\
	(X3) = (*RK) ^							\
	   ( (uint32_t) sbox[ ( (Y3)       ) & 0xFF ]       ) ^		\
	   ( (uint32_t) sbox[ ( (Y4) >>  8 ) & 0xFF ] <<  8 ) ^		\
	   ( (uint32_t) sbox[ ( (Y6) >> 16 ) & 0xFF ] << 16 ) ^		\
	   ( (uint32_t) sbox[ ( (Y7) >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	(X4) = (*RK) ^							\
	    ( (uint32_t) sbox[ ( (Y4)	  ) & 0xFF ]	   ) ^		\
	    ( (uint32_t) sbox[ ( (Y5) >>  8 ) & 0xFF ] <<  8 ) ^	\
	    ( (uint32_t) sbox[ ( (Y7) >> 16 ) & 0xFF ] << 16 ) ^	\
	    ( (uint32_t) sbox[ ( (Y0) >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	(X5) = (*RK) ^							\
	    ( (uint32_t) sbox[ ( (Y5)       ) & 0xFF ]       ) ^	\
	    ( (uint32_t) sbox[ ( (Y6) >>  8 ) & 0xFF ] <<  8 ) ^	\
	    ( (uint32_t) sbox[ ( (Y0) >> 16 ) & 0xFF ] << 16 ) ^	\
	    ( (uint32_t) sbox[ ( (Y1) >> 24 ) & 0xFF ] << 24 ); RK++;	\
									\
	(X6) = (*RK) ^							\
	   ( (uint32_t) sbox[ ( (Y6)       ) & 0xFF ]       ) ^		\
	   ( (uint32_t) sbox[ ( (Y7) >>  8 ) & 0xFF ] <<  8 ) ^		\
	   ( (uint32_t) sbox[ ( (Y1) >> 16 ) & 0xFF ] << 16 ) ^		\
	   ( (uint32_t) sbox[ ( (Y2) >> 24 ) & 0xFF ] << 24 ); RK++; 	\
									\
	(X7) = (*RK) ^							\
	   ( (uint32_t) sbox[ ( (Y7)       ) & 0xFF ]       ) ^		\
	   ( (uint32_t) sbox[ ( (Y0) >>  8 ) & 0xFF ] <<  8 ) ^		\
	   ( (uint32_t) sbox[ ( (Y2) >> 16 ) & 0xFF ] << 16 ) ^		\
	   ( (uint32_t) sbox[ ( (Y3) >> 24 ) & 0xFF ] << 24 ); RK++;	\
} while(0);

static int rijndael_enc(const rijndael_table_ctx *ctx, const uint8_t *data_in, uint8_t *data_out)
{
	uint32_t i;
	/* Our local key and state */
	uint32_t *RK, X0, X1, X2, X3, X4, X5, X6, X7, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7;
	int ret = -1;

	if((ctx == NULL) || (data_in == NULL) || (data_out == NULL)){
		goto err;
	}
	/* Sanity check for array access */
	if((4 * (ctx->Nb) * (ctx->Nr + 1)) > sizeof(ctx->rk)){
		goto err;
	}
	if((ctx->Nr >> 1) == 0){
		goto err;
	}

	/* Go for AES rounds */
	/* Load key and message */
	RK = (uint32_t*)ctx->rk;
	GET_UINT32_LE(X0, data_in,  0 ); X0 ^= (*RK); RK++;
	GET_UINT32_LE(X1, data_in,  4 ); X1 ^= (*RK); RK++;
	GET_UINT32_LE(X2, data_in,  8 ); X2 ^= (*RK); RK++;
	GET_UINT32_LE(X3, data_in, 12 ); X3 ^= (*RK); RK++;
	if(ctx->Nb == 8){
		GET_UINT32_LE(X4, data_in, 16 ); X4 ^= (*RK); RK++;
		GET_UINT32_LE(X5, data_in, 20 ); X5 ^= (*RK); RK++;
		GET_UINT32_LE(X6, data_in, 24 ); X6 ^= (*RK); RK++;
		GET_UINT32_LE(X7, data_in, 28 ); X7 ^= (*RK); RK++;
	}
	else{
		/* NOTE: to avoid false positive "use of uninitialized value" in gcc */
		X4 = X5 = X6 = X7 = 0;
	}
	/**/
	if(ctx->Nb == 4){
		for(i = ((ctx->Nr >> 1) - 1); i > 0; i--){
			RIJNDAEL_ENC_ROUND_16(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
			RIJNDAEL_ENC_ROUND_16(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
		}
		RIJNDAEL_ENC_ROUND_16(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
		/* Last round without Mixcolumns */
		RIJNDAEL_ENC_LASTROUND_16(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
	}
	else if(ctx->Nb == 8){
		for(i = ((ctx->Nr >> 1) - 1); i > 0; i--){
			RIJNDAEL_ENC_ROUND_32(Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7, X0, X1, X2, X3, X4, X5, X6, X7);
			RIJNDAEL_ENC_ROUND_32(X0, X1, X2, X3, X4, X5, X6, X7, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7);
		}
		RIJNDAEL_ENC_ROUND_32(Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7, X0, X1, X2, X3, X4, X5, X6, X7);
		/* Last round without Mixcolumns */
		RIJNDAEL_ENC_LASTROUND_32(X0, X1, X2, X3, X4, X5, X6, X7, Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7);
	}
	else{
		ret = -1;
		goto err;
	}

	PUT_UINT32_LE(X0, data_out,  0 );
	PUT_UINT32_LE(X1, data_out,  4 );
	PUT_UINT32_LE(X2, data_out,  8 );
	PUT_UINT32_LE(X3, data_out, 12 );
	if(ctx->Nb == 8){
		PUT_UINT32_LE(X4, data_out, 16 );
		PUT_UINT32_LE(X5, data_out, 20 );
		PUT_UINT32_LE(X6, data_out, 24 );
		PUT_UINT32_LE(X7, data_out, 28 );
	}
	/* Clean stuff */
	memset(&X0, 0, sizeof(X0)); memset(&X1, 0, sizeof(X1));
	memset(&X2, 0, sizeof(X2)); memset(&X3, 0, sizeof(X3));
	memset(&X4, 0, sizeof(X4)); memset(&X5, 0, sizeof(X5));
	memset(&X6, 0, sizeof(X6)); memset(&X7, 0, sizeof(X7));
	/* */
	memset(&Y0, 0, sizeof(Y0)); memset(&Y1, 0, sizeof(Y1));
	memset(&Y2, 0, sizeof(Y2)); memset(&Y3, 0, sizeof(Y3));
	memset(&Y4, 0, sizeof(Y4)); memset(&Y5, 0, sizeof(Y5));
	memset(&Y6, 0, sizeof(Y6)); memset(&Y7, 0, sizeof(Y7));

	ret = 0;

err:
	return ret;
}

/* ==== Public APIs ===== */

int aes128_table_setkey_enc(rijndael_table_ctx *ctx, const uint8_t key[16])
{
	return rijndael_setkey_enc(ctx, key, AES128);
}

int aes256_table_setkey_enc(rijndael_table_ctx *ctx, const uint8_t key[32])
{
	return rijndael_setkey_enc(ctx, key, AES256);
}

int rijndael256_table_setkey_enc(rijndael_table_ctx *ctx, const uint8_t key[32])
{
	return rijndael_setkey_enc(ctx, key, RIJNDAEL_256_256);
}

int aes128_table_enc(const rijndael_table_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16])
{
	int ret = -1;

	if(ctx->rtype != AES128){
		goto err;
	}

	ret = rijndael_enc(ctx, data_in, data_out);
	
err:
	return ret;
}

int aes128_table_enc_x2(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16])
{
        int ret;

        ret  = aes128_table_enc(ctx1, plainText1, cipherText1);
        ret |= aes128_table_enc(ctx2, plainText2, cipherText2);

        return ret;
}

int aes128_table_enc_x4(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const rijndael_table_ctx *ctx3, const rijndael_table_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16])
{
        int ret;

        ret  = aes128_table_enc(ctx1, plainText1, cipherText1);
        ret |= aes128_table_enc(ctx2, plainText2, cipherText2);
        ret |= aes128_table_enc(ctx3, plainText3, cipherText3);
        ret |= aes128_table_enc(ctx4, plainText4, cipherText4);

        return ret;
}

int aes256_table_enc(const rijndael_table_ctx *ctx, const uint8_t data_in[16], uint8_t data_out[16])
{
	int ret = -1;

	if(ctx->rtype != AES256){
		goto err;
	}

	ret = rijndael_enc(ctx, data_in, data_out);
	
err:
	return ret;
}

int aes256_table_enc_x2(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const uint8_t plainText1[16], const uint8_t plainText2[16], uint8_t cipherText1[16], uint8_t cipherText2[16])
{
        int ret;

        ret  = aes256_table_enc(ctx1, plainText1, cipherText1);
        ret |= aes256_table_enc(ctx2, plainText2, cipherText2);

        return ret;
}

int aes256_table_enc_x4(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const rijndael_table_ctx *ctx3, const rijndael_table_ctx *ctx4,
                const uint8_t plainText1[16], const uint8_t plainText2[16], const uint8_t plainText3[16], const uint8_t plainText4[16],
                uint8_t cipherText1[16], uint8_t cipherText2[16], uint8_t cipherText3[16], uint8_t cipherText4[16])
{
        int ret;

        ret  = aes256_table_enc(ctx1, plainText1, cipherText1);
        ret |= aes256_table_enc(ctx2, plainText2, cipherText2);
        ret |= aes256_table_enc(ctx3, plainText3, cipherText3);
        ret |= aes256_table_enc(ctx4, plainText4, cipherText4);

        return ret;
}

int rijndael256_table_enc(const rijndael_table_ctx *ctx, const uint8_t data_in[32], uint8_t data_out[32])
{
	int ret = -1;

	if(ctx->rtype != RIJNDAEL_256_256){
		goto err;
	}

	ret = rijndael_enc(ctx, data_in, data_out);
	
err:
	return ret;
}

int rijndael256_table_enc_x2(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2,
                        const uint8_t plainText1[32], const uint8_t plainText2[32],
                        uint8_t cipherText1[32], uint8_t cipherText2[32])
{
        int ret;

        ret  = rijndael256_table_enc(ctx1, plainText1, cipherText1);
        ret |= rijndael256_table_enc(ctx2, plainText2, cipherText2);

        return ret;

}


int rijndael256_table_enc_x4(const rijndael_table_ctx *ctx1, const rijndael_table_ctx *ctx2, const rijndael_table_ctx *ctx3, const rijndael_table_ctx *ctx4,
                const uint8_t plainText1[32], const uint8_t plainText2[32], const uint8_t plainText3[32], const uint8_t plainText4[32],
                uint8_t cipherText1[32], uint8_t cipherText2[32], uint8_t cipherText3[32], uint8_t cipherText4[32])
{
        int ret;

        ret  = rijndael256_table_enc(ctx1, plainText1, cipherText1);
        ret |= rijndael256_table_enc(ctx2, plainText2, cipherText2);
        ret |= rijndael256_table_enc(ctx3, plainText3, cipherText3);
        ret |= rijndael256_table_enc(ctx4, plainText4, cipherText4);

        return ret;
}

#else /* !RIJNDAEL_TABLE */
/*
 * Dummy definition to avoid the empty translation unit ISO C warning
 */
typedef int dummy;
#endif
