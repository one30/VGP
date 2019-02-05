#include <iostream>
#include <algorithm>
#include <cstdbool>
#include <cstdint>
#include <cstring>
#include <cassert>
#include "bdap.h"
#include "rand.h"
#include "ed25519.h"
#include "curve25519.h"
#include "shake256.h"
#include "aes256ctr.h"
#include "aes256gcm.h"
#include "utils.h"
#include "bdap_error.h"

#define DO_TEST(name, func)   \
    std::cout << name << std::flush; \
    if (false == func) { \
        std::cout << "FAIL" << std::endl; \
        return -1; \
    } else { \
        std::cout << "PASS" << std::endl; \
    }

bool randomPositiveTest()
{
    int32_t index;
    const int32_t kNumberOfKeys = 10;

    // a. Create a random key seeds and use them to create a vchPubKeys variable with ten Ed25519
    //    public keys and a vchPrivKeySeeds variable with ten private key seeds.
    vCharVector vchPubKeys(kNumberOfKeys, CharVector(ED25519_PUBLIC_KEY_SIZE));
    vCharVector vchPrivKeySeeds(kNumberOfKeys, CharVector(ED25519_PRIVATE_KEY_SEED_SIZE));
    for (index = 0; index < kNumberOfKeys; ++index)
    {
        CharVector vchSeed(ED25519_PRIVATE_KEY_SEED_SIZE);
        CharVector vchPrivateKey(ED25519_PRIVATE_KEY_SIZE);

        bdap_randombytes(vchSeed.data(), ED25519_PRIVATE_KEY_SEED_SIZE);
        ed25519_seeded_keypair(vchPubKeys[index].data(), vchPrivateKey.data(), vchSeed.data());
        vchPrivKeySeeds[index] = vchSeed;
    }

    // b. Create a random length of string between 1000-5000 characters for the vchData variable.
    uint16_t vchDataLength = 0;
    bdap_randombytes(reinterpret_cast<uint8_t *>(&vchDataLength), sizeof(uint16_t));
    vchDataLength = 1000 + (vchDataLength & 0x0FFF);
    CharVector vchData(vchDataLength);
    bdap_randombytes(vchData.data(), vchDataLength);

    // c. Call EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage). Make sure
    //    EncryptBDAPData returns true and strErrorMessage is empty.
    bool encryptStatus = false;
    std::string strErrorMessage("N/A");
    CharVector vchCipherText;
    encryptStatus = EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage);
    assert(encryptStatus == true);
    assert(0 == strErrorMessage.compare(std::string(bdap_error_message[BDAP_SUCCESS])));

    // d. For each private key seed in vchPrivKeySeeds, use the key seed call
    //    DecryptBDAPData(vchPrivKeySeed, vchCipherText, vchDecryptData, strErrorMessage)
    // e. Unit test passes if EncryptBDAPData returns true, each vchDecryptData == vchData,
    //   all calls to DecryptBDAPData return true and strErrorMessage is empty.
    for (index = 0; index < kNumberOfKeys; ++index)
    {
        strErrorMessage = "N/A";
        CharVector vchDecrypted;
        bool decryptStatus = DecryptBDAPData(vchPrivKeySeeds[index], vchCipherText, vchDecrypted, strErrorMessage);
        assert(decryptStatus == true);
        assert(0 == strErrorMessage.compare(std::string(bdap_error_message[BDAP_SUCCESS])));
        assert(0 == memcmp(vchData.data(), vchDecrypted.data(), vchData.size()));
    }

    return true;
}

bool hardCodedPositiveTest()
{
    std::size_t index;

    // a. Use a hardcoded list of known Ed25519 seeds for vchPrivKeySeeds and
    //    use it to create vcPubKeys.
    const vCharVector vchPrivKeySeeds{
        { 0x83, 0x19, 0xc1, 0xab, 0xae, 0xd6, 0x15, 0x71, 
          0xc1, 0x8f, 0xae, 0x4c, 0x20, 0x45, 0x8e, 0x5e, 
          0xcd, 0x65, 0x46, 0x0e, 0xed, 0x36, 0xec, 0xdb, 
          0x29, 0x57, 0x52, 0x8d, 0x5c, 0xda, 0xcc, 0x19
        },
        { 0xf4, 0xc2, 0xbf, 0xe1, 0x00, 0x78, 0x06, 0x4c,
          0xcf, 0x8d, 0x31, 0x5f, 0x3b, 0x54, 0xe2, 0xea,
          0xb7, 0x15, 0x1c, 0x00, 0x11, 0x55, 0x89, 0x89,
          0x04, 0xf4, 0x6d, 0x79, 0xd4, 0xf9, 0x1e, 0x2b
        },
        { 0xab, 0xfa, 0x14, 0x83, 0xc8, 0x69, 0xae, 0x1a,
          0x74, 0xa2, 0xcc, 0xb3, 0x11, 0x20, 0x81, 0x6c,
          0x99, 0x3b, 0x47, 0x98, 0xda, 0x11, 0x03, 0x9c,
          0x96, 0xc0, 0x47, 0xa2, 0x5e, 0xbb, 0x7c, 0xca
        },
        { 0x60, 0x6b, 0xef, 0xb8, 0x3d, 0x27, 0x43, 0x4b,
          0x96, 0x01, 0x40, 0x7a, 0xcd, 0xb1, 0x74, 0x11,
          0x6e, 0xf6, 0x96, 0x8d, 0x41, 0x1c, 0x49, 0xfd,
          0x88, 0x92, 0xe1, 0x4d, 0x54, 0x8a, 0xbf, 0x64
        },
        { 0x87, 0xb3, 0xb7, 0x8b, 0xa0, 0x99, 0x50, 0x4c,
          0x0b, 0x2d, 0x1a, 0xc4, 0xa0, 0xf8, 0x0a, 0x74,
          0xe7, 0x09, 0x8d, 0x3b, 0xa9, 0x02, 0xbb, 0xa4,
          0x7f, 0x7f, 0x8d, 0xa7, 0x32, 0x18, 0x00, 0x83
        },
        { 0x0b, 0xd8, 0x89, 0x39, 0x1c, 0x4e, 0x1b, 0xe3,
          0x94, 0x70, 0xee, 0x03, 0xb8, 0x1f, 0x40, 0xc6,
          0x5e, 0xcc, 0x81, 0x66, 0xaf, 0x8d, 0x07, 0x66,
          0xc6, 0x54, 0x37, 0xb0, 0x65, 0xef, 0x94, 0xc6
        },
        { 0xa4, 0xd9, 0xe6, 0x15, 0xc9, 0xc6, 0x44, 0xc5,
          0x23, 0x17, 0x55, 0x30, 0x41, 0x08, 0x58, 0xe2,
          0x58, 0x80, 0xda, 0xcf, 0x0c, 0xa4, 0x0c, 0x2b,
          0x5a, 0xd6, 0x80, 0x88, 0xfd, 0x3d, 0xec, 0xf6
        },
        { 0x3e, 0xf1, 0x3a, 0xcf, 0xde, 0x50, 0x55, 0xe3,
          0xdd, 0x51, 0x95, 0xd1, 0x38, 0x7d, 0xa8, 0xda,
          0x0e, 0x72, 0xd8, 0x74, 0xcb, 0xca, 0x00, 0xfa,
          0xea, 0x4f, 0x6f, 0x95, 0x00, 0xa1, 0x3a, 0x07
        },
        { 0x59, 0x8c, 0x32, 0xae, 0xb9, 0xab, 0x55, 0xeb,
          0x1c, 0xc6, 0x44, 0x68, 0x84, 0xaa, 0x52, 0x80,
          0x84, 0xf8, 0x83, 0xf2, 0xdb, 0x08, 0x80, 0x9a,
          0x0b, 0xae, 0xd8, 0xfa, 0x18, 0x69, 0xf6, 0x84
        },
        { 0xd7, 0xd0, 0x78, 0xd8, 0xd2, 0x33, 0xaa, 0x6e,
          0x19, 0xca, 0x04, 0x06, 0x35, 0xe1, 0x41, 0x24,
          0x0b, 0x34, 0x74, 0x28, 0x85, 0x58, 0x58, 0xcb,
          0xd7, 0xf9, 0x0c, 0xcd, 0x21, 0xda, 0x9a, 0x10
        }
    };
    vCharVector vchPubKeys(vchPrivKeySeeds.size(), CharVector(ED25519_PUBLIC_KEY_SIZE));
    for (index = 0; index < vchPrivKeySeeds.size(); ++index)
    {
        ed25519_public_key_from_private_key_seed(vchPubKeys[index].data(), vchPrivKeySeeds[index].data());
    }

    // b. Use a hardcoded string with 2500 characters for the vchData variable.
    const CharVector vchData{
        0xbf,0x9c,0x39,0x58,0xb4,0x09,0x47,0xe8,0x5e,0x57,0x43,0x6e,0xbd,0xd2,0xbb,0x4d,
        0xb4,0x59,0x37,0xa4,0x70,0x78,0xc8,0xaf,0x3d,0x88,0x77,0x59,0x84,0x16,0x87,0x58,
        0xef,0xcc,0x68,0x35,0x58,0x59,0x8d,0xee,0x86,0x2b,0x14,0xaa,0xf3,0x3b,0xd6,0xdd,
        0xd0,0x05,0x6c,0x3e,0xa9,0xd5,0x58,0xe7,0x3f,0x97,0x68,0x2b,0x75,0xa1,0xb4,0x5c,
        0xb7,0xa2,0xc6,0x07,0x84,0xd7,0x3b,0x6c,0x65,0x66,0x55,0x90,0x82,0xf8,0xd8,0x64,
        0xad,0xf8,0xe9,0x48,0x32,0x83,0x97,0xa7,0x9f,0xad,0x77,0x09,0xe0,0x30,0x9f,0xcf,
        0x0f,0xf2,0x10,0x71,0xc2,0xe7,0x7c,0x60,0x00,0xd3,0x20,0x79,0x5e,0xa4,0x59,0x2e,
        0xc0,0x8f,0xb5,0x24,0xaa,0xb8,0xe7,0x3f,0xdc,0x49,0xe8,0x95,0xc9,0x9b,0x11,0x9d,
        0xea,0x7a,0xd5,0x04,0x5e,0xd7,0x8b,0xfc,0x1d,0x54,0x05,0xd6,0xcd,0x30,0xf8,0x30,
        0x06,0x1f,0xa5,0x8d,0x41,0x1b,0x7f,0x0e,0x0b,0xe5,0x90,0x9c,0xdd,0xf7,0x21,0xe9,
        0xa2,0x32,0x05,0xa1,0xaf,0x01,0xbb,0xac,0xbb,0x58,0xe6,0xc8,0xc9,0x0f,0x7c,0xf4,
        0x76,0x6f,0xef,0x86,0x37,0x96,0x4c,0x11,0x55,0x9e,0xd9,0x6b,0x2e,0x65,0xeb,0xa3,
        0x1a,0xc6,0x68,0xd5,0xe6,0x4c,0x74,0xe7,0x4b,0x49,0xf9,0x88,0x86,0xe7,0x9f,0x44,
        0xdd,0xab,0xdf,0x92,0x3b,0x8d,0x52,0x79,0x36,0x7e,0x55,0x40,0x84,0xb7,0x1a,0x50,
        0x39,0x3a,0x0d,0x5c,0xa0,0x72,0xb2,0xc3,0xfd,0xf8,0xcd,0xba,0x1e,0x93,0x9e,0x5c,
        0x25,0x7c,0xa6,0x2f,0x4f,0xa5,0xc8,0x96,0x64,0x08,0x5d,0xa4,0x30,0x1b,0xce,0xcb,
        0x7c,0x38,0x05,0xc4,0x00,0x3f,0xb7,0x5a,0x98,0xa3,0x83,0xa6,0x33,0xb4,0x75,0xbf,
        0xe7,0xe9,0xe3,0xe2,0xac,0xbb,0xb9,0xf1,0xff,0x4a,0xfb,0xbe,0x40,0x9f,0x5b,0xbd,
        0x5e,0xdb,0x1d,0xfa,0x91,0xaa,0xf3,0xfc,0x79,0x7d,0x15,0xe6,0x35,0x2e,0x65,0x18,
        0xf0,0x31,0x2c,0xa7,0x60,0xff,0x81,0xf9,0x42,0x4b,0x66,0x78,0x73,0xd9,0xc9,0xe5,
        0xe8,0xc8,0x96,0x38,0x83,0x09,0x69,0xdf,0x9b,0x54,0x41,0xe7,0xdf,0x0a,0x5b,0x3d,
        0xfd,0xbf,0x5e,0xcf,0x4c,0x19,0x74,0xf2,0x4a,0x00,0xf9,0xd3,0xf7,0xdb,0xbb,0x98,
        0x41,0x56,0x77,0x39,0x88,0x81,0xd9,0xba,0x04,0x63,0x51,0x7c,0x05,0x01,0x5e,0x0d,
        0x91,0xb1,0xf1,0x35,0x90,0x24,0x6f,0x9b,0x26,0x66,0x0a,0x9b,0x02,0x31,0x42,0xa5,
        0xbd,0x44,0x59,0xe2,0xba,0xf7,0xbf,0x7e,0x8f,0x08,0x04,0xc1,0xbe,0xd9,0xb4,0x07,
        0xf8,0xf3,0x15,0x27,0xa4,0xf0,0x39,0x7f,0x17,0x56,0xa0,0x3f,0x39,0x70,0x5e,0xc1,
        0x55,0x10,0x11,0x6a,0x9f,0x91,0x76,0x06,0x03,0xc4,0xd6,0xf2,0xfc,0xf3,0x71,0x42,
        0x44,0x5f,0x95,0xa2,0xf0,0xcb,0x57,0x97,0xe7,0x37,0xe6,0xcd,0x9c,0xbd,0x5f,0x9b,
        0x21,0x3f,0x55,0x50,0xcb,0x8b,0xd5,0x62,0x8d,0x73,0xe8,0x55,0x40,0x75,0xe8,0x00,
        0x17,0x03,0x44,0xd6,0x73,0xb7,0x56,0x47,0xd3,0xb8,0x28,0x97,0x0e,0x90,0xb2,0xc1,
        0x4f,0x9a,0xc4,0x91,0xfe,0x09,0x64,0x8e,0x19,0x44,0x07,0x66,0x7d,0x68,0x21,0xda,
        0x59,0xcc,0x8d,0x80,0x77,0x1f,0x4b,0x48,0x85,0x94,0xbb,0x56,0x4a,0x34,0x2c,0xcb,
        0x21,0x60,0xdc,0xbb,0xfa,0xc0,0x2b,0x10,0x08,0x01,0x57,0xc7,0xb1,0xda,0xfd,0x12,
        0xae,0xed,0x3e,0x8e,0x14,0x04,0x69,0x96,0xcb,0x3b,0x04,0x49,0x9b,0xe6,0x31,0xbd,
        0x3a,0xea,0xef,0xdd,0xfa,0xe8,0x7a,0xd1,0x61,0x2d,0x06,0x3f,0x4d,0x40,0x14,0xb4,
        0xbe,0x99,0xc3,0x0e,0xf2,0x9b,0x46,0x48,0xc9,0x65,0x32,0x65,0xc1,0xe3,0x90,0x79,
        0x04,0x4a,0x63,0x0a,0xe5,0xad,0x95,0xb6,0xc9,0x62,0x2a,0x68,0x88,0xde,0xce,0xc3,
        0x73,0xbe,0x00,0x91,0xc3,0xfa,0x90,0x12,0xbc,0x34,0xe7,0x76,0xb8,0xa3,0xea,0x3b,
        0x59,0x84,0xd9,0x9d,0x26,0xbf,0x26,0x02,0x19,0x09,0x0b,0x6c,0x27,0x9c,0xea,0xa3,
        0x20,0xc8,0x49,0x43,0xbf,0x67,0x30,0x66,0xd8,0x54,0xec,0x76,0x59,0xfa,0x99,0x63,
        0x60,0x40,0x42,0xdd,0xfa,0xaa,0x56,0xf0,0xd4,0x6d,0xe5,0xc1,0x24,0xc2,0xbc,0xac,
        0x89,0x25,0xae,0xf0,0xc3,0x04,0x4f,0xa3,0x94,0xd0,0x7f,0xeb,0x4c,0xa1,0x8f,0xfa,
        0x69,0x09,0x5a,0xb0,0xc7,0x92,0x7b,0x03,0x9b,0x22,0xa5,0xce,0x01,0xef,0x4c,0xeb,
        0xb1,0x97,0xae,0x2c,0x8e,0x91,0xf1,0x03,0x9e,0x41,0x24,0x51,0xc3,0x0e,0x85,0x5b,
        0xcc,0x3c,0x65,0x2a,0x0e,0xea,0x7e,0x8d,0x98,0x23,0xb5,0x63,0x86,0x60,0x01,0x51,
        0x3d,0xf7,0x4d,0x7d,0xeb,0xca,0x75,0x72,0xe3,0x90,0x34,0x00,0x86,0x92,0xd4,0xcc,
        0xff,0x5e,0xed,0xe3,0xb0,0x19,0x45,0xdb,0x06,0x8b,0xa2,0x6e,0x67,0x79,0x76,0xc5,
        0x65,0xc1,0xa5,0x0f,0x3e,0x23,0x9a,0xac,0xf7,0xa9,0xb3,0xa5,0x15,0x14,0xde,0x50,
        0x06,0xe3,0x11,0x1b,0xb1,0x97,0xaf,0x9c,0x7d,0xce,0x4f,0x2b,0xf7,0xec,0xe3,0x93,
        0x2c,0xa1,0xac,0xda,0x35,0x04,0x43,0xb2,0xb7,0xe3,0xb1,0x3c,0x73,0x2d,0x88,0x8b,
        0x07,0x67,0x7a,0xe6,0x4e,0x7f,0x8b,0x70,0xce,0xfe,0xff,0x96,0xdf,0xe1,0x7b,0x97,
        0xef,0xd6,0x21,0xb4,0x60,0xfe,0x42,0x9f,0x82,0x99,0xd1,0x90,0x8b,0xf4,0xd4,0xcc,
        0xf3,0x80,0x12,0x24,0x36,0x64,0x72,0xc3,0x06,0xd8,0x30,0xb0,0x85,0xf4,0x20,0x66,
        0x55,0x84,0x38,0x85,0x2e,0xe3,0xb5,0xf5,0x2a,0x4e,0x8f,0x48,0x10,0x8b,0xa4,0xda,
        0x88,0x9c,0xab,0x15,0xea,0x5f,0x42,0x6f,0xcd,0xf3,0x09,0x27,0x75,0x65,0x22,0xbb,
        0xf7,0x98,0xc3,0x47,0x22,0xe9,0x38,0x1a,0xa7,0x6d,0xed,0xd0,0x3c,0x12,0xf9,0xdd,
        0x7b,0xcd,0x6c,0x1a,0xc2,0xec,0xfe,0xb6,0x49,0xa6,0x79,0xed,0x93,0xdd,0x5f,0x02,
        0x9a,0xaf,0x31,0x6d,0x9d,0xc4,0x49,0x10,0xf4,0x5c,0xf5,0x9d,0x24,0xd5,0xf4,0x38,
        0xae,0xd3,0x32,0x8a,0x81,0x4f,0xdf,0x96,0x3b,0xbb,0x62,0xbf,0xb3,0x6d,0x8a,0xb9,
        0xe6,0x5c,0x5d,0x58,0xe2,0xb5,0x81,0xf5,0x94,0xf7,0x49,0xdd,0x21,0x06,0xd1,0x66,
        0x5f,0xdb,0x05,0x54,0x18,0xa9,0x0d,0x2e,0x5b,0x5e,0xc7,0x43,0xc4,0x98,0x9f,0x30,
        0x33,0xfc,0x9c,0x2f,0xd1,0x19,0xf1,0xfa,0x22,0x8a,0xe9,0x96,0x3e,0x0c,0xb5,0xa3,
        0x56,0x75,0xcb,0x19,0x30,0x15,0x1c,0x8e,0xd3,0xdc,0xe3,0x13,0xc7,0x58,0xb1,0xe0,
        0x04,0x52,0xef,0xb7,0xa0,0x48,0xb5,0xe8,0xfe,0x0f,0xe3,0x1c,0x2b,0x17,0x64,0x5a,
        0x3a,0x6f,0xfe,0x45,0xec,0x2e,0x6f,0xba,0x33,0x5c,0xc8,0x25,0x92,0x6c,0xa2,0xe5,
        0x97,0x84,0xdd,0xf9,0x50,0xb9,0xa4,0x24,0xc2,0x95,0xb1,0x9c,0xbe,0x09,0xfe,0x24,
        0x61,0xa5,0xee,0x73,0x38,0x73,0xd5,0x70,0x71,0xad,0x39,0xdb,0x43,0x03,0x75,0xf1,
        0x7a,0x6a,0xe7,0x60,0x22,0x7d,0x7f,0xb1,0x3b,0x5b,0xc5,0xbe,0xcb,0xb0,0x1d,0xd7,
        0x2b,0x00,0x2e,0x37,0xe0,0x14,0x17,0x57,0xb9,0xac,0x4d,0xe3,0xdf,0xdf,0xc0,0x79,
        0x61,0x3b,0x29,0x4c,0x70,0xe9,0xf1,0x9a,0x9d,0xa5,0x26,0xd2,0x79,0xcf,0xe7,0xb8,
        0xd1,0xcf,0xf9,0xed,0x89,0xc7,0x46,0x2f,0x48,0xff,0x8d,0xb1,0x4a,0x8f,0xa6,0xd2,
        0x0f,0xc7,0x6b,0x7f,0x8a,0x5d,0x7d,0x67,0x0e,0x3f,0xec,0x5a,0xaa,0x1b,0x82,0x47,
        0xe2,0xd8,0x31,0x4f,0xef,0x5b,0xa7,0x77,0xe7,0xe1,0xdb,0x89,0xe7,0x10,0x50,0x3e,
        0x68,0xa8,0xab,0xd3,0xf7,0x9c,0xd2,0xd3,0x39,0x9d,0x3f,0xdf,0x36,0x87,0x48,0x25,
        0xef,0xed,0x27,0xd1,0x69,0xab,0x1e,0x05,0xd1,0xcd,0x4c,0xcd,0x1a,0x4c,0xa1,0x55,
        0x0f,0x75,0x1d,0xe7,0xf0,0xb9,0xb6,0xed,0xb3,0x6e,0x65,0x3e,0x8b,0x74,0x1b,0x8c,
        0x67,0x68,0x80,0x7d,0x23,0x54,0x6e,0x12,0xd5,0x60,0x44,0xe4,0xd8,0x92,0x13,0x1e,
        0x25,0x29,0xdf,0xdd,0xef,0x9a,0x34,0xd0,0x78,0xb4,0xfe,0x51,0xb8,0x83,0xa1,0x08,
        0x6e,0xb2,0x43,0xd2,0xcd,0x06,0xba,0xed,0x89,0x82,0xb4,0x8c,0xb9,0xdd,0x12,0x88,
        0xa9,0xed,0x59,0xef,0x8b,0xe3,0xdf,0x5d,0xf0,0x1e,0xfa,0x3d,0xdb,0xf0,0x4d,0x16,
        0xec,0xcd,0x84,0xb2,0x62,0x32,0xc7,0xfa,0xf0,0x93,0x24,0x30,0x6b,0xdc,0xc3,0xd8,
        0x2e,0xc2,0xf4,0x59,0xdd,0x15,0x1c,0x9c,0x84,0x95,0xfc,0xaa,0xd2,0x41,0xc7,0x7a,
        0xaa,0x37,0x0e,0xbf,0x58,0x3f,0x7a,0x60,0xd4,0x51,0xe5,0x04,0xf8,0x3b,0xa1,0x6f,
        0x31,0x56,0x07,0x49,0xbc,0xbc,0x2f,0x74,0x62,0x53,0xc3,0xfa,0xb3,0x08,0x75,0x8a,
        0x10,0xe5,0x6b,0xdc,0x5e,0x71,0xc3,0x03,0xc0,0x56,0x1b,0xdd,0x87,0x6b,0x5a,0xf7,
        0xd1,0xdf,0x71,0xd5,0xe4,0xa7,0xdb,0x61,0x9a,0xe9,0x6c,0xda,0x3a,0x4f,0x7b,0xc1,
        0x29,0x9c,0xc3,0xa9,0xba,0x74,0x83,0xd5,0x15,0xa8,0x4d,0x4d,0x9d,0xb9,0x9c,0x7e,
        0xa5,0xbb,0x70,0xb9,0xff,0xe7,0x41,0xf0,0xbf,0xea,0x87,0x33,0xfb,0xbf,0x1b,0x95,
        0x68,0x15,0xbd,0x7a,0xa2,0x00,0xbb,0x58,0x53,0x88,0x32,0x44,0x0e,0x5b,0x03,0x3c,
        0xfc,0x7f,0x45,0x17,0xb8,0x7c,0xd8,0x76,0xa0,0xe5,0xc7,0x85,0x58,0x59,0x97,0x20,
        0x1b,0x86,0xe3,0xdf,0xa4,0x31,0x6d,0x5c,0x82,0xb6,0x0c,0x2a,0xd5,0x78,0x8a,0xb3,
        0x09,0x58,0x27,0xe4,0x04,0x13,0x50,0xc0,0x8d,0x89,0x83,0x47,0x90,0x41,0xb6,0x86,
        0x6a,0x3a,0x47,0x80,0x20,0xc3,0xab,0x01,0x5a,0xe0,0x58,0xcc,0xa7,0x2d,0xfd,0x9b,
        0x60,0xe4,0x13,0x84,0xef,0xc3,0x66,0x34,0xc3,0x76,0xc5,0x7d,0xb0,0x90,0x73,0xa5,
        0xb3,0x7a,0x8c,0x0e,0xef,0x5a,0x71,0x9a,0x17,0xb9,0xf7,0x09,0x17,0xdd,0x2b,0xee,
        0x8c,0x01,0x5d,0x42,0x6f,0xd9,0x45,0x7e,0x29,0x6f,0x30,0xc6,0x44,0xd2,0x34,0x11,
        0x84,0xb3,0x42,0x2b,0x96,0x71,0xa7,0x79,0xe7,0xd1,0x61,0xea,0x0a,0x28,0x3d,0x35,
        0xd9,0x04,0xdf,0xb6,0x1c,0x78,0xc1,0x5a,0x81,0x4b,0x6c,0x2c,0x55,0x08,0xc5,0x2e,
        0xe5,0x47,0x4c,0x07,0xaf,0xfa,0xda,0x2e,0xee,0x5e,0xb9,0xec,0x9b,0xd3,0x1b,0x66,
        0x6f,0x0c,0xea,0x2d,0x50,0x74,0xb2,0xb7,0x4d,0x21,0xce,0x40,0x6b,0xf4,0x5d,0x9a,
        0x04,0xad,0xe6,0x08,0x52,0xa4,0xd9,0x10,0xc6,0x4a,0x1d,0x23,0x0d,0x9f,0x53,0x7f,
        0xed,0xe9,0xfc,0x19,0xb2,0x7e,0xae,0x5e,0xdd,0x4f,0x83,0x7f,0x2e,0x6e,0x14,0xd3,
        0x28,0x56,0xdb,0x4f,0xb3,0x7b,0x50,0x7a,0xa2,0xdd,0xe3,0xc7,0x5c,0x59,0xa1,0x9a,
        0xd3,0x54,0x8e,0x42,0x1c,0xfd,0x37,0x82,0x62,0x8f,0xd5,0x91,0xda,0x6f,0x26,0x47,
        0x40,0xa1,0x98,0xb3,0x78,0x50,0xfc,0x89,0x57,0xe3,0xba,0xaf,0x11,0xe3,0x43,0x72,
        0x12,0x05,0xbe,0x84,0xf9,0x11,0x33,0xfa,0x43,0x2f,0x15,0x30,0x6b,0x43,0xa8,0xef,
        0x9d,0x76,0x03,0x89,0x26,0x71,0x09,0x3b,0x58,0x60,0xa5,0x83,0x1c,0xfa,0x7a,0x83,
        0x6e,0x1d,0xd1,0x80,0xeb,0xb8,0xb6,0x19,0xa6,0xcd,0x62,0x00,0x6e,0xcf,0xab,0x35,
        0x71,0x6f,0x1e,0x73,0xd5,0x21,0x03,0x4c,0xc8,0x1e,0x63,0x89,0xb7,0xc5,0x23,0x36,
        0xee,0xc8,0x5c,0xe1,0xd3,0x85,0x9b,0xd1,0x08,0xc5,0xa8,0x39,0x61,0x81,0xe0,0x53,
        0xcc,0x1f,0x13,0xc6,0xd2,0xff,0xfc,0x74,0xa2,0x56,0xcc,0xa9,0x72,0xf5,0x23,0xf7,
        0x33,0x7b,0xc9,0xf3,0x88,0xd4,0xcc,0x23,0x3e,0x30,0x13,0x66,0xdb,0xcf,0x2c,0x03,
        0x64,0x27,0xd9,0x4b,0x40,0xd7,0xa6,0x39,0xe4,0x37,0x9f,0xcf,0xdd,0x8d,0xc8,0x9a,
        0xe6,0x6e,0x67,0xa1,0xaa,0xef,0xda,0x23,0x5b,0x8f,0x9c,0x28,0xb4,0xdf,0xae,0x84,
        0x84,0x33,0xa0,0x58,0x40,0x55,0x8e,0x65,0xd1,0x20,0x02,0xb1,0x9f,0x15,0xbc,0xfc,
        0x2c,0x2f,0x0b,0x8d,0xd7,0xf3,0x5d,0xc8,0x53,0x55,0xb7,0xb0,0x43,0x21,0x47,0xfb,
        0x1b,0x4b,0xd3,0xbe,0x2b,0xc9,0x7a,0x50,0xcb,0x72,0x93,0x1a,0xde,0x27,0x61,0xe0,
        0x33,0x4b,0x09,0x7f,0x7d,0x9b,0x4a,0xdb,0x2e,0xb7,0x20,0x6b,0x9a,0x0f,0x79,0x73,
        0xbb,0xd2,0x07,0x1a,0x75,0xa2,0xe4,0xbd,0xaa,0x04,0xb2,0x98,0xc4,0x57,0x9d,0xba,
        0x09,0xaf,0x72,0x93,0x0a,0x0f,0xe2,0xb6,0x3e,0xc8,0xa3,0xe8,0x11,0x14,0x93,0xba,
        0x56,0xfe,0x50,0xcd,0xe7,0x90,0x9c,0xfc,0x74,0xa6,0x7d,0xf7,0x5d,0x1d,0x5b,0x09,
        0xd8,0x85,0x3c,0x82,0x3c,0x03,0x35,0xb7,0x51,0x8f,0xed,0xab,0xa1,0x4f,0xf9,0xaa,
        0x03,0x19,0x8f,0x6f,0x53,0x13,0xe2,0xb7,0x8b,0x91,0x2b,0x9e,0xab,0x3a,0xf3,0x12,
        0x93,0x46,0x7c,0xb8,0x72,0x69,0x56,0x95,0x03,0x4c,0x9b,0x12,0x5b,0x49,0xbf,0xac,
        0xdb,0x35,0xd2,0x08,0xc4,0x31,0x47,0x5a,0xe6,0x50,0xb9,0xe4,0xcd,0xb0,0xfa,0x4f,
        0x6b,0xf9,0x90,0x93,0x54,0x99,0x14,0xe7,0x8f,0xf4,0x5a,0xa6,0x1b,0xce,0xa2,0x98,
        0xc1,0x84,0x71,0x33,0xaf,0x3a,0x2d,0xe0,0x96,0xdc,0xb6,0xca,0xaf,0x66,0xa0,0x94,
        0xa0,0x4a,0xb2,0xc8,0x97,0x28,0xc1,0x8b,0xa0,0x9f,0xc0,0xa3,0x07,0x22,0x13,0x07,
        0x4d,0xe1,0xd5,0xe0,0xac,0xbb,0x46,0x9a,0xe2,0x60,0x1e,0x34,0x91,0xc8,0xf6,0x6e,
        0xa1,0x7a,0x6e,0x6e,0x70,0x9a,0x01,0xdf,0x44,0xa6,0x06,0xac,0xf2,0xc8,0x0a,0xf2,
        0x32,0x75,0xfc,0x06,0xe7,0x1f,0x8e,0x53,0x6f,0x14,0xb7,0x63,0x0d,0x78,0x5c,0xa1,
        0xb7,0x61,0xe4,0x40,0x82,0x28,0xe9,0xae,0x3b,0x2a,0x38,0x13,0xd9,0x71,0xe4,0x5f,
        0x8a,0x17,0xb2,0xbf,0x9c,0xbf,0x43,0xbf,0xef,0x86,0xc2,0xbc,0x8f,0x47,0xef,0x11,
        0x20,0x0a,0x14,0xbb,0x8c,0x7a,0x63,0xd0,0x91,0x5d,0x63,0xd8,0x44,0xdf,0xd7,0x5f,
        0x23,0xe3,0xb5,0x52,0x0f,0x57,0x10,0x31,0x10,0x29,0x1c,0xfc,0xfb,0xe3,0xe0,0xa4,
        0xf5,0x82,0x9a,0xd3,0x55,0x75,0xd9,0xe9,0xd5,0x3b,0xaa,0x7b,0x6a,0x90,0x3e,0x70,
        0x77,0x0b,0x2a,0x5c,0xfe,0x1d,0x71,0x24,0xa1,0xb8,0x06,0x2e,0x95,0xf8,0x1c,0x8a,
        0x76,0x64,0x33,0xa6,0x5d,0xa4,0xf5,0xd5,0xd7,0xb6,0xe1,0x72,0x41,0x31,0x08,0xd2,
        0x74,0xf9,0xa8,0x14,0x8f,0x64,0xca,0x74,0x33,0x8a,0x2c,0xc6,0x5d,0xc1,0x8f,0x6f,
        0x5d,0x73,0x20,0x58,0x6b,0x08,0xb1,0x05,0x9f,0x5a,0x20,0x0f,0x1b,0xe3,0x26,0xcf,
        0xab,0x8f,0x0c,0xcc,0x60,0x09,0x50,0xf8,0x32,0xed,0x8c,0x02,0x33,0x29,0x44,0xeb,
        0xf5,0x70,0x91,0x75,0x3e,0x56,0xde,0x42,0x53,0x17,0xaa,0x1e,0xd2,0x0e,0x4e,0x9d,
        0x04,0x13,0x47,0xc6,0xd8,0x46,0x48,0x86,0x70,0xbd,0xae,0x7a,0x75,0xdb,0xe9,0x7f,
        0x43,0x97,0xed,0x9d,0x8a,0x57,0x24,0xb6,0x89,0x82,0xcb,0x52,0xb7,0x59,0x72,0x38,
        0x7b,0xca,0x22,0x82,0xbd,0x8f,0xd1,0xcd,0xed,0xa3,0xd1,0xb0,0x39,0x78,0x49,0xd2,
        0x25,0x0f,0xc1,0x55,0xbc,0x95,0xe5,0x23,0xd0,0x80,0x87,0x62,0x7b,0x82,0x50,0xd5,
        0xbd,0x81,0x05,0x4d,0x50,0xca,0xd5,0xf9,0x7b,0x5a,0x71,0xa2,0x0b,0xa7,0x23,0xe6,
        0xf5,0xea,0xfc,0x83,0xc3,0xac,0xbd,0x0f,0x6f,0x89,0xfc,0x66,0x95,0x84,0x68,0x8e,
        0xa5,0xc1,0x23,0x02,0x16,0x67,0x75,0x44,0x6c,0x43,0x59,0xe5,0xde,0xb9,0xc9,0x1e,
        0x4b,0x35,0xe2,0x20,0xde,0xf7,0xe6,0xb2,0xff,0xee,0x3d,0xba,0x8e,0xa7,0x67,0x23,
        0xad,0x55,0xac,0xc1,0xb8,0xee,0xcf,0xae,0x95,0x18,0xc1,0x8a,0xe2,0x4d,0x0b,0x63,
        0x0c,0xea,0xf6,0xbb,0x1b,0x8d,0x5e,0xf4,0x76,0x8c,0xc6,0x88,0xb2,0xfe,0xbf,0xfd,
        0xd9,0x6d,0x0c,0xb6,0x68,0x6a,0xdb,0x1e,0x7d,0x46,0xee,0x30,0x5e,0xae,0x85,0xd2,
        0xf8,0x6a,0x86,0xc4,0x39,0x59,0x2a,0x04,0xb1,0x26,0xd5,0x7d,0xb9,0x22,0xe0,0xb0,
        0xaf,0xdc,0x7c,0xfe,0x6e,0xbe,0x8c,0x33,0x29,0xad,0x0c,0xa0,0xb7,0x92,0x72,0x02,
        0x3c,0xc9,0xde,0xdd,0x0e,0xcf,0x60,0x55,0x88,0x64,0xe1,0xb5,0x9a,0xf4,0xea,0x56,
        0xa2,0xd2,0x7b,0x7d
    };

    // c. Call EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage). Make sure
    //    EncryptBDAPData returns true and strErrorMessage is empty.
    bool encryptStatus = false;
    std::string strErrorMessage("N/A");
    CharVector vchCipherText;
    encryptStatus = EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage);
    assert(encryptStatus == true);
    assert(0 == strErrorMessage.compare(std::string(bdap_error_message[BDAP_SUCCESS])));

    // d. For each private key seed in vchPrivKeySeeds, use the key seed call
    //    DecryptBDAPData(vchPrivKeySeed, vchCipherText, vchDecryptData, strErrorMessage)
    // e. Unit test passes if EncryptBDAPData returns true, each vchDecryptData == vchData,
    //   all calls to DecryptBDAPData return true and strErrorMessage is empty.
    for (index = 0; index < vchPubKeys.size(); ++index)
    {
        strErrorMessage = "N/A";
        CharVector vchDecrypted;
        bool decryptStatus = DecryptBDAPData(vchPrivKeySeeds[index], vchCipherText, vchDecrypted, strErrorMessage);
        assert(decryptStatus == true);
        assert(0 == strErrorMessage.compare(std::string(bdap_error_message[BDAP_SUCCESS])));
        assert(0 == memcmp(vchData.data(), vchDecrypted.data(), vchData.size()));
    }

    return true;
}

bool randomNegativeTest()
{
    int32_t index;
    const int32_t kNumberOfKeys = 10;

    // a. Create 10 random key seeds and use them to create a vchPubKeys variable
    vCharVector vchPubKeys(kNumberOfKeys, CharVector(ED25519_PUBLIC_KEY_SIZE));
    for (index = 0; index < kNumberOfKeys; index++)
    {
        CharVector vchSeed(ED25519_PRIVATE_KEY_SEED_SIZE);

        bdap_randombytes(vchSeed.data(), ED25519_PRIVATE_KEY_SEED_SIZE);
        ed25519_public_key_from_private_key_seed(vchPubKeys[index].data(), vchSeed.data());
    }

    // b. Create a random length of string between 1000-5000 characters for the vchData variable.
    uint16_t vchDataLength = 0;
    bdap_randombytes(reinterpret_cast<uint8_t *>(&vchDataLength), sizeof(uint16_t));
    vchDataLength = 1000 + (vchDataLength & 0x0FFF);
    CharVector vchData(vchDataLength);
    bdap_randombytes(vchData.data(), vchDataLength);

    // c. Call EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage). Make sure
    //    EncryptBDAPData returns true.
    bool encryptStatus = false;
    std::string strErrorMessage("N/A");
    CharVector vchCipherText;
    encryptStatus = EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage);
    assert(encryptStatus == true);

    // d. Create ten new random key seeds vchPrivKeySeed and use them to call
    //    DecryptBDAPData(vchPrivKeySeed, vchCipherText, vchDecryptData, strErrorMessage)
    // e. Unit test passes if EncryptBDAPData returns true, all calls to DecryptBDAPData
    //    return false and strErrorMessage is NOT empty.
    int32_t numFailures = 0;
    for (index = 0; index < kNumberOfKeys; index++)
    {
        CharVector vchPrivKeySeed(ED25519_PRIVATE_KEY_SEED_SIZE);
        bdap_randombytes(vchPrivKeySeed.data(), ED25519_PRIVATE_KEY_SEED_SIZE);

        std::string strErrorMessage("");
        CharVector vchDecryptedData;
        if (false == DecryptBDAPData(vchPrivKeySeed, vchCipherText, vchDecryptedData, strErrorMessage)
                    && strErrorMessage.size() > 0)
        {
            numFailures++;
        }
    }
    assert(numFailures == kNumberOfKeys);

    return true;
}

bool decryptLastValueNegativeTest()
{
    int32_t index;
    const int32_t kNumberOfKeys = 3;
    const int32_t kFingerprintSize = 7;
    const int32_t kSecretSize = 32;
    const int32_t kEncryptedSecretSize = 32;
    const int32_t kBufferSize = 3*CURVE25519_PUBLIC_KEY_SIZE;
    const int32_t kKeyIVSize = AES256CTR_KEY_SIZE + AES256CTR_IV_SIZE;
    const int32_t kKeyNonceSize = AES256GCM_KEY_SIZE + AES256GCM_NONCE_SIZE;

    // a. Create 3 random key seeds and use them to create a vchPubKeys variable
    vCharVector vchPubKeys(kNumberOfKeys, CharVector(ED25519_PUBLIC_KEY_SIZE));
    for (index = 0; index < kNumberOfKeys; index++)
    {
        CharVector vchSeed(ED25519_PRIVATE_KEY_SEED_SIZE);

        bdap_randombytes(vchSeed.data(), ED25519_PRIVATE_KEY_SEED_SIZE);
        ed25519_public_key_from_private_key_seed(vchPubKeys[index].data(), vchSeed.data());
    }

    // b. Create a random length of string between 1000-5000 characters for the vchData variable.
    uint16_t vchDataLength = 0;
    bdap_randombytes(reinterpret_cast<uint8_t *>(&vchDataLength), sizeof(uint16_t));
    vchDataLength = 1000 + (vchDataLength & 0x0FFF);
    CharVector vchData(vchDataLength);
    bdap_randombytes(vchData.data(), vchDataLength);

    // c. Call EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage). Make sure
    //    EncryptBDAPData returns true and strErrorMessage is empty.
    bool encryptStatus = false;
    std::string strErrorMessage("N/A");
    CharVector vchCipherText;
    encryptStatus = EncryptBDAPData(vchPubKeys, vchData, vchCipherText, strErrorMessage);
    assert(encryptStatus == true);
    assert(0 == strErrorMessage.compare(std::string(bdap_error_message[BDAP_SUCCESS])));

    // d. Parse vchCipherText and extract the payload ciphertext into variable vchLastValue.
    CharVector::const_iterator iterator = vchCipherText.cbegin();
    uint16_t numRecipient = iterator[0] + 256 * iterator[1];
    iterator += 2;
    CharVector vchEphemeralPublicKey(CURVE25519_PUBLIC_KEY_SIZE);
    vchEphemeralPublicKey.assign(iterator, iterator + CURVE25519_PUBLIC_KEY_SIZE);
    iterator += CURVE25519_PUBLIC_KEY_SIZE;
    iterator += (numRecipient * (kFingerprintSize + kEncryptedSecretSize));
    CharVector vchLastValue(kEncryptedSecretSize);
    vchLastValue.assign(iterator - kEncryptedSecretSize, iterator);
    CharVector vchGCMCipher(vchDataLength + AES256GCM_TAG_SIZE);
    vchGCMCipher.assign(iterator, vchCipherText.cend());

    // e. Try to decrypt vchLastValue using three new randomly generated Curve25519 private key.
    for (index = 0; index < kNumberOfKeys; index++)
    {
        CharVector randomCurve25519PublicKey(CURVE25519_PUBLIC_KEY_SIZE, 0);
        CharVector randomCurve25519PrivateKey(CURVE25519_PRIVATE_KEY_SIZE, 0);

        CharVector vchQ(CURVE25519_POINT_SIZE, 0);
        curve25519_random_keypair(randomCurve25519PublicKey.data(),
                                  randomCurve25519PrivateKey.data());
        bool result = curve25519_dh(vchQ.data(),
                                    randomCurve25519PrivateKey.data(),
                                    vchEphemeralPublicKey.data());
        assert(result == true);

        CharVector vchBuffer(kBufferSize, 0);
        CharVector::iterator outputIterator = vchBuffer.begin();
        iterator = vchQ.begin();
        std::copy(iterator, iterator + vchQ.size(), outputIterator);
        outputIterator += vchQ.size();
        iterator = randomCurve25519PublicKey.begin();
        std::copy(iterator, iterator + randomCurve25519PublicKey.size(), outputIterator);
        outputIterator += randomCurve25519PublicKey.size();
        iterator = vchEphemeralPublicKey.begin();
        std::copy(iterator, iterator + vchEphemeralPublicKey.size(), outputIterator);

        CharVector vchKeyIV(kKeyIVSize, 0);
        result = (0 == shake256(vchKeyIV.data(), kKeyIVSize, vchBuffer.data(), vchBuffer.size()));
        assert(result == true);

        CharVector vchSecret(kSecretSize, 0);
        size_t secretSize = 0;
        result = (0 == aes256ctr_decrypt(vchSecret.data(),
                                         &secretSize,
                                         vchLastValue.data(),
                                         vchLastValue.size(),
                                         (vchKeyIV.data() + AES256CTR_KEY_SIZE),
                                         vchKeyIV.data()));
        assert(secretSize == vchSecret.size());
        assert(result == true);

        CharVector vchKeyNonce(kKeyIVSize, 0);
        result = (0 == shake256(vchKeyNonce.data(), kKeyNonceSize, vchSecret.data(), vchSecret.size()));
        assert(result == true);

        CharVector plaintext(vchDataLength);
        size_t plaintextSize = 0;
        result = (0 != aes256gcm_decrypt(plaintext.data(),
                                         &plaintextSize,
                                         vchGCMCipher.data(),
                                         vchGCMCipher.size(),
                                         NULL,
                                         0,
                                         (vchKeyNonce.data() + AES256GCM_KEY_SIZE),
                                         vchKeyNonce.data()));
        assert(result == true);
        assert(plaintextSize == 0);
    }

    // f. Unit test passes if EncryptBDAPData returns true, parsing and extraction is successful,
    //    and all attempts to decrypt vchLastValue fail.

    return true;
}

int main(void)
{
    DO_TEST("Random positive test: ", randomPositiveTest())

    DO_TEST("Hard-coded positive test: ", hardCodedPositiveTest())

    DO_TEST("Random negative test: ", randomNegativeTest())

    DO_TEST("Decrypt last-value negative test: ", decryptLastValueNegativeTest())

    return 0;
}