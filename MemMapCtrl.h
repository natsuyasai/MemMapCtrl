/**
* @file MemMapCtrl.h
* @brief �������}�b�s���O�̈�Ǘ��N���X
* @author
*/

#ifndef MEM_MAP_CTRL_H
#define MEM_MAP_CTRL_H

#include <stdint.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// �萔��`
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// BIT�A�N�Z�X�p
union BitInf
{
	unsigned char byte[4];
	struct
	{
		unsigned int b00 : 1;      // byte0,bit0
		unsigned int b01 : 1;      // byte0,bit1
		unsigned int b02 : 1;      // byte0,bit2
		unsigned int b03 : 1;      // byte0,bit3
		unsigned int b04 : 1;      // byte0,bit4
		unsigned int b05 : 1;      // byte0,bit5
		unsigned int b06 : 1;      // byte0,bit6
		unsigned int b07 : 1;      // byte0,bit7
		unsigned int b10 : 1;      // byte1,bit0
		unsigned int b11 : 1;      // byte1,bit1
		unsigned int b12 : 1;      // byte1,bit2
		unsigned int b13 : 1;      // byte1,bit3
		unsigned int b14 : 1;      // byte1,bit4
		unsigned int b15 : 1;      // byte1,bit5
		unsigned int b16 : 1;      // byte1,bit6
		unsigned int b17 : 1;      // byte1,bit7
		unsigned int b20 : 1;      // byte2,bit0
		unsigned int b21 : 1;      // byte2,bit1
		unsigned int b22 : 1;      // byte2,bit2
		unsigned int b23 : 1;      // byte2,bit3
		unsigned int b24 : 1;      // byte2,bit4
		unsigned int b25 : 1;      // byte2,bit5
		unsigned int b26 : 1;      // byte2,bit6
		unsigned int b27 : 1;      // byte2,bit7
		unsigned int b30 : 1;      // byte3,bit0
		unsigned int b31 : 1;      // byte3,bit1
		unsigned int b32 : 1;      // byte3,bit2
		unsigned int b33 : 1;      // byte3,bit3
		unsigned int b34 : 1;      // byte3,bit4
		unsigned int b35 : 1;      // byte3,bit5
		unsigned int b36 : 1;      // byte3,bit6
		unsigned int b37 : 1;      // byte3,bit7
	}bit;
};

// �}�b�s���O�����߂�l���
struct GetMapReslt
{
    uint32_t aligned;  // �A���C�����g������̃T�C�Y
    char* unmapAddr;   // unmap���w��p�A�h���X
    void* mmapAddr;    // �I�t�Z�b�g������̃}�b�v�h�̈�擪�A�h���X
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// �N���X��`
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class MemMapCtrl
{
public:
	// �R���X�g���N�^
	MemMapCtrl();
	// �f�X�g���N�^
	~MemMapCtrl();

	//---------------------------------
	// private�ϐ�
	//---------------------------------
private:
	// �}�b�s���O�̈�
	char* mMemMappedRegionP;
	// �������f�o�C�X
	int mMemDev;
	// �A���C�����g������T�C�Y
	uint32_t mMemMappedSize;
	// unmap�A�h���X
	char* mUnmapAddrP;
    // �}�b�s���O�̈�x�[�X�A�h���X
    uint32_t mMemMapBaseAddr;

	//---------------------------------
	// public�֐�
	//---------------------------------
public:
	// ������
	bool init(uint32_t memryBaseAddr, uint32_t memMapSize);
	// �O��l�ێ���������
	void setDataHoldingPrevVal(unsigned int setAddr, const BitInf& setBitInf, const unsigned char dataP[4]);
	// �f�[�^��������
	void setMemData(unsigned int setAddr, int size, const unsigned char* dataP);
	// �f�[�^�擾
	void getMemData(unsigned char* dataP, unsigned int getAddr, int size);
	// char4byte�f�[�^��int�ϊ�
	unsigned int convBinToVal(unsigned char bin[4]);
	// char4byte�f�[�^��short�ϊ�
	unsigned short convBinToVal2Byte(unsigned char bin[2]);
	// int��char4byte�f�[�^�ϊ�
	void convValToBin(unsigned char bin[4], unsigned int val);
	// �������}�b�s���O
    GetMapReslt getMapedAddr(int, uint32_t, uint32_t);
	//---------------------------------
	// private�֐�
	//---------------------------------
private:
	// �A�h���X�w��
	char* getOffsetMapedAdder(unsigned int);
	// �}�X�N�f�[�^����
	void createMaskData(unsigned char[4], const BitInf&);
};

#endif // MEM_MAP_CTRL_H
// EOF