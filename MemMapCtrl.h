/**
* @file MemMapCtrl.h
* @brief メモリマッピング領域管理クラス
* @author
*/

#ifndef MEM_MAP_CTRL_H
#define MEM_MAP_CTRL_H

#include <stdint.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 定数定義
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// BITアクセス用
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

// マッピング処理戻り値情報
struct GetMapReslt
{
    uint32_t aligned;  // アライメント調整後のサイズ
    char* unmapAddr;   // unmap時指定用アドレス
    void* mmapAddr;    // オフセット調整後のマップド領域先頭アドレス
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// クラス定義
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class MemMapCtrl
{
public:
	// コンストラクタ
	MemMapCtrl();
	// デストラクタ
	~MemMapCtrl();

	//---------------------------------
	// private変数
	//---------------------------------
private:
	// マッピング領域
	char* mMemMappedRegionP;
	// メモリデバイス
	int mMemDev;
	// アライメント調整後サイズ
	uint32_t mMemMappedSize;
	// unmapアドレス
	char* mUnmapAddrP;
    // マッピング領域ベースアドレス
    uint32_t mMemMapBaseAddr;

	//---------------------------------
	// public関数
	//---------------------------------
public:
	// 初期化
	bool init(uint32_t memryBaseAddr, uint32_t memMapSize);
	// 前回値保持書き込み
	void setDataHoldingPrevVal(unsigned int setAddr, const BitInf& setBitInf, const unsigned char dataP[4]);
	// データ書き込み
	void setMemData(unsigned int setAddr, int size, const unsigned char* dataP);
	// データ取得
	void getMemData(unsigned char* dataP, unsigned int getAddr, int size);
	// char4byteデータ→int変換
	unsigned int convBinToVal(unsigned char bin[4]);
	// char4byteデータ→short変換
	unsigned short convBinToVal2Byte(unsigned char bin[2]);
	// int→char4byteデータ変換
	void convValToBin(unsigned char bin[4], unsigned int val);
	// メモリマッピング
    GetMapReslt getMapedAddr(int, uint32_t, uint32_t);
	//---------------------------------
	// private関数
	//---------------------------------
private:
	// アドレス指定
	char* getOffsetMapedAdder(unsigned int);
	// マスクデータ生成
	void createMaskData(unsigned char[4], const BitInf&);
};

#endif // MEM_MAP_CTRL_H
// EOF