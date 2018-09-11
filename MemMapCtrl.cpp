/**
* @file MemMapCtrl.cpp
* @brief �������}�b�s���O�̈�Ǘ��N���X
* @author 
*/

//********************************************************************
// include
//********************************************************************

#include "MemMapCtrl.h"
#include <string.h>
// mmap�֘A
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>


// �f�t�H���g�T�C�Y
constexpr int DEF_ACCESS_SIZE = 4;
// �}�b�s���O�p�f�[�^�t�@�C���p�X
constexpr char MEM_DEV_PATH[] = "/dev/mem";

/// <summary>
/// �R���X�g���N�^
/// </summary>
MemMapCtrl::MemMapCtrl()
    : mMemMappedRegionP()
    , mMemDev()
    , mMemMappedSize()
    , mUnmapAddrP()
{
}

/// <summary>
/// �f�X�g���N�^
/// </summary>
MemMapCtrl::~MemMapCtrl()
{
    // �A���}�b�v
    munmap(mUnmapAddrP, mMemMappedSize);
}

/// <summary>
/// ������
/// </summary>
/// <param name="mapedBaseAddr">�}�b�s���O�̈�擪�A�h���X</param>
/// <param name="mapSize">�}�b�s���O�T�C�Y</param>
/// <returns></returns>
bool MemMapCtrl::init(uint32_t mapedBaseAddr, uint32_t mapSize)
{
    // �������f�o�C�X�I�[�v��
    if ((mMemDev = open(MEM_DEV_PATH, O_RDWR, 0)) < 0)
    {
        // �I�[�v�����s
        printf("Memory Dev Open Err\n");
        return false;
    }
    // �������̈�}�b�s���O
    GetMapReslt mmapRslt = getMapedAddr(mMemDev, mapedBaseAddr, mapSize);

    // �}�b�s���O���ʊm�F
    if (mMemMappedRegionP == nullptr)
    {
        printf("MemMap Failed\n");
        return false;
    }
    mMemMappedRegionP = (char*)mmapRslt.mmapAddr;
    mMemMappedSize = mmapRslt.aligned;
    mUnmapAddrP = mmapRslt.unmapAddr;
    mMemMapBaseAddr = mapedBaseAddr;

    return true;
}


/// <summary>
/// �O��l�ێ���������
/// </summary>
/// <param name="setAddr">�������ݐ�A�h���X</param>
/// <param name="setBitInf">�}�X�N���(�������ݑΏ�bit��1�Ƃ���)</param>
/// <param name="dataP">�������݃f�[�^(4byte)</param>
void MemMapCtrl::setDataHoldingPrevVal(unsigned int setAddr, const BitInf& setBitInf, const unsigned char dataP[4])
{

    unsigned char maskData[DEF_ACCESS_SIZE] = {0};     // �}�X�N�f�[�^
    // �����ݏꏊ�擾
    char* setAddrP = getOffsetMapedAdder(setAddr);

    // �O�l�擾
    uint32_t beforeData = *((uint32_t*)setAddrP);

    // �}�X�N�f�[�^����(�����ݑΏ�bit�݂̂�0�Ƃ����f�[�^)
    createMaskData(maskData, setBitInf);

    // �����ݑΏ�bit�݂̂�0�ɗ��Ƃ����O�l�𐶐����A
    // ���������O�l�̏����ݑΏ�bit�������݃f�[�^�ɕύX����B
    //               �����ݑΏ�bit�݂̂�0�ɗ��Ƃ����l | �����݃f�[�^
    uint32_t writeData = ((beforeData & *((uint32_t*)maskData)) | *((uint32_t*)dataP));

    // �}�b�s���O�̈�֏�����
    *((unsigned long *)setAddrP) = writeData;

    // ���W�X�^�֓���
    msync(setAddrP, DEF_ACCESS_SIZE, MS_SYNC);
}


/// <summary>
/// �f�[�^��������
/// </summary>
/// <param name="setAddr">�������ݐ�A�h���X</param>
/// <param name="size">�������݃T�C�Y</param>
/// <param name="dataP">�������݃f�[�^</param>
void MemMapCtrl::setMemData(unsigned int setAddr, int size , const unsigned char* dataP)
{
    // �����ݏꏊ�擾
    void* setAddrP = getOffsetMapedAdder(setAddr);
    // �}�b�s���O�̈�֏�����
    memcpy(setAddrP, dataP, size);
    // ���W�X�^�֓���
    msync(setAddrP, size, MS_SYNC);
}


/// <summary>
/// �f�[�^�擾
/// </summary>
/// <param name="dataP">�擾�f�[�^</param>
/// <param name="getAddr">�擾��A�h���X</param>
/// <param name="size">�擾�T�C�Y</param>
void MemMapCtrl::getMemData(unsigned char* dataP, unsigned int getAddr, int size)
{
    // �ǂݎ��ꏊ�擾
    void* setAddrP = getOffsetMapedAdder(getAddr);
    // �}�b�s���O�̈悩��擾
    memcpy(dataP, setAddrP, size);
}


/// <summary>
/// �o�C�i���f�[�^�����l�ϊ�
/// 4byte�̃o�C�i���f�[�^�𐔒l�ɕϊ�����
/// </summary>
/// <param name="bin">�o�C�i���f�[�^</param>
/// <returns>���l�f�[�^</returns>
unsigned int MemMapCtrl::convBinToVal(unsigned char bin[4])
{
    unsigned int val = 0;

    // 1byte�����l�̕ϊ����A�Y��bit�܂ŃV�t�g������B
    unsigned int byte4 = 0;
    byte4 = bin[3];
    byte4 <<= 24;
    unsigned int byte3 = 0;
    byte3 = bin[2];
    byte3 <<= 16;
    unsigned int byte2 = 0;
    byte2 = bin[1];
    byte2 <<= 8;
    // �ebyte����ɂ܂Ƃ߂�
    val |= byte4;
    val |= byte3;
    val |= byte2;
    val |= bin[0];

    return val;
}

/// <summary>
/// �o�C�i���f�[�^�����l�ϊ�
/// 2byte�̃o�C�i���f�[�^�𐔒l�ɕϊ�����
/// </summary>
/// <param name="bin">�o�C�i���f�[�^</param>
/// <returns>���l�f�[�^</returns>
unsigned short MemMapCtrl::convBinToVal2Byte(unsigned char bin[2])
{
    unsigned int val = 0;

    // 1byte�����l�̕ϊ����A�Y��bit�܂ŃV�t�g������B
    unsigned int byte2 = 0;
    byte2  = bin[1];
    byte2 <<= 8;
    // �ebyte����ɂ܂Ƃ߂�
    val |= byte2;
    val |= bin[0];

    return (unsigned short)val;
}


/// <summary>
/// ���l���o�C�i���f�[�^�ϊ�
/// ���l��4byte�̃o�C�i���f�[�^�ɕϊ�����
/// </summary>
/// <param name="bin">�o�C�i���f�[�^</param>
/// <param name="val">���l</param>
void MemMapCtrl::convValToBin(unsigned char bin[4], unsigned int val)
{
    // 0�N���A
    memset(bin, 0, DEF_ACCESS_SIZE);

    // 1byte�����Ɏ��o���ď�������
    bin[3] = (unsigned char)((val & 0xFF000000) >> 24);
    bin[2] = (unsigned char)((val & 0x00FF0000) >> 16);
    bin[1] = (unsigned char)((val & 0x0000FF00) >> 8);
    bin[0] = (unsigned char)(val & 0x000000FF);
}


/// <summary>
/// �������}�b�s���O�̈�擾
/// </summary>
/// <param name="aligned">�A���C�����g������̃T�C�Y</param>
/// <param name="unmap_addr">mmap�̖߂�l�̃A�h���X</param>
/// <param name="mem_device">�������f�o�C�X</param>
/// <param name="addr">�}�b�s���O���s���擪�A�h���X</param>
/// <param name="size">�}�b�s���O�T�C�Y</param>
/// <returns>�}�b�s���O����</returns>
GetMapReslt MemMapCtrl::getMapedAddr(int mem_device, uint32_t addr, uint32_t size)
{
    GetMapReslt rslt = {};
    void* mem;
    char* aligned_vaddr;
    uint32_t aligned_paddr;
    uint32_t aligned_size;

    // 4byte���E�ɍ��킹��i4�����Ȃ����O�̋��E�ɁA5�ȏ�8�����Ȃ�����̋��E�ɍ��킹��j
    int unit = 4; //32bit fixed
    uint32_t count = size / unit;
    uint32_t paddr;

    // Align address to access size for write to dest
    paddr = addr;
    paddr &= 0xFFFFFFFC;
    if (size < 4096)
    {
        aligned_paddr = paddr & ~(4096 - 1);
        aligned_size = paddr - aligned_paddr + (count * unit);
        aligned_size = (aligned_size + 4096 - 1) & ~(4096 - 1);
    }
    else
    {
        aligned_paddr = paddr & ~(8192 - 1);
        aligned_size = paddr - aligned_paddr + (count * unit);
        aligned_size = (aligned_size + 8192 - 1) & ~(8192 - 1);
    }

    // �}�b�s���O
    aligned_vaddr = (char*)mmap(NULL, aligned_size, (PROT_WRITE | PROT_READ), MAP_SHARED, mem_device, aligned_paddr);
    if (aligned_vaddr == MAP_FAILED) {
        printf("Error: get_maped_addr: Error mapping address\n");
        return nullptr;
    }
    // �I�t�Z�b�g����
    mem = (void *)(aligned_vaddr + (paddr - aligned_paddr));
    
    // �߂�l�ݒ�
    rslt.aligned = aligned_size;
    rslt.unmapAddr = aligned_vaddr;
    rslt.mmapAddr = mem;
}


//--------------------------------------------------------------------
//    ���\�b�h�� : getOffsetMapedAdder
//      ���{�ꖼ :
//        �T  �v : �w��A�h���X�ɑΉ������}�b�s���O�̈�A�h���X���擾����
//    �p�����[�^ : [IN] addr : �擾�������A�h���X
//      ���A��� : �}�b�s���O�̈�A�h���X
//        ��  �l : �Ȃ�
//      �X�V���� :
//        ��  �t   ��  �e
//    2018/mm/dd   �V�K�쐬
//--------------------------------------------------------------------
char* MemMapCtrl::getOffsetMapedAdder(unsigned int addr)
{
    return (char*)(mMemMappedRegionP + (addr - mMemMapBaseAddr));
}



//--------------------------------------------------------------------
//    ���\�b�h�� : createMaskData
//      ���{�ꖼ : �}�X�N�f�[�^����
//        �T  �v : �����ݎ��̃}�X�N�f�[�^�𐶐�����B
//    �p�����[�^ : [OUT]  maskData  �}�X�N�f�[�^
//               : [IN ] bitInf    �}�X�N�p��bit���
//      ���A��� :
//        ��  �l : �Ȃ�
//      �X�V���� :
//        ��  �t   ��  �e
//    2018/mm/dd   �V�K�쐬
//--------------------------------------------------------------------
void MemMapCtrl::createMaskData(unsigned char maskData[4], const  BitInf& bitInf)
{
    // �ebit�𔽓]�����A�����ݑΏ�bit�ȊO��1�Ƃ����}�X�N�f�[�^�𐶐�����
    BitInf flipBit;
    flipBit.bit.b07 = (!bitInf.bit.b07);
    flipBit.bit.b06 = (!bitInf.bit.b06);
    flipBit.bit.b05 = (!bitInf.bit.b05);
    flipBit.bit.b04 = (!bitInf.bit.b04);
    flipBit.bit.b03 = (!bitInf.bit.b03);
    flipBit.bit.b02 = (!bitInf.bit.b02);
    flipBit.bit.b01 = (!bitInf.bit.b01);
    flipBit.bit.b00 = (!bitInf.bit.b00);
    maskData[0] = flipBit.byte[0];
    flipBit.bit.b17 = (!bitInf.bit.b17);
    flipBit.bit.b16 = (!bitInf.bit.b16);
    flipBit.bit.b15 = (!bitInf.bit.b15);
    flipBit.bit.b14 = (!bitInf.bit.b14);
    flipBit.bit.b13 = (!bitInf.bit.b13);
    flipBit.bit.b12 = (!bitInf.bit.b12);
    flipBit.bit.b11 = (!bitInf.bit.b11);
    flipBit.bit.b10 = (!bitInf.bit.b10);
    maskData[1] = flipBit.byte[1];
    flipBit.bit.b27 = (!bitInf.bit.b27);
    flipBit.bit.b26 = (!bitInf.bit.b26);
    flipBit.bit.b25 = (!bitInf.bit.b25);
    flipBit.bit.b24 = (!bitInf.bit.b24);
    flipBit.bit.b23 = (!bitInf.bit.b23);
    flipBit.bit.b22 = (!bitInf.bit.b22);
    flipBit.bit.b21 = (!bitInf.bit.b21);
    flipBit.bit.b20 = (!bitInf.bit.b20);
    maskData[2] = flipBit.byte[2];
    flipBit.bit.b37 = (!bitInf.bit.b37);
    flipBit.bit.b36 = (!bitInf.bit.b36);
    flipBit.bit.b35 = (!bitInf.bit.b35);
    flipBit.bit.b34 = (!bitInf.bit.b34);
    flipBit.bit.b33 = (!bitInf.bit.b33);
    flipBit.bit.b32 = (!bitInf.bit.b32);
    flipBit.bit.b31 = (!bitInf.bit.b31);
    flipBit.bit.b30 = (!bitInf.bit.b30);
    maskData[3] = flipBit.byte[3];
}


// EOF