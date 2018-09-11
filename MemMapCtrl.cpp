/**
* @file MemMapCtrl.cpp
* @brief メモリマッピング領域管理クラス
* @author 
*/

//********************************************************************
// include
//********************************************************************

#include "MemMapCtrl.h"
#include <string.h>
// mmap関連
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>


// デフォルトサイズ
constexpr int DEF_ACCESS_SIZE = 4;
// マッピング用データファイルパス
constexpr char MEM_DEV_PATH[] = "/dev/mem";

/// <summary>
/// コンストラクタ
/// </summary>
MemMapCtrl::MemMapCtrl()
    : mMemMappedRegionP()
    , mMemDev()
    , mMemMappedSize()
    , mUnmapAddrP()
{
}

/// <summary>
/// デストラクタ
/// </summary>
MemMapCtrl::~MemMapCtrl()
{
    // アンマップ
    munmap(mUnmapAddrP, mMemMappedSize);
}

/// <summary>
/// 初期化
/// </summary>
/// <param name="mapedBaseAddr">マッピング領域先頭アドレス</param>
/// <param name="mapSize">マッピングサイズ</param>
/// <returns></returns>
bool MemMapCtrl::init(uint32_t mapedBaseAddr, uint32_t mapSize)
{
    // メモリデバイスオープン
    if ((mMemDev = open(MEM_DEV_PATH, O_RDWR, 0)) < 0)
    {
        // オープン失敗
        printf("Memory Dev Open Err\n");
        return false;
    }
    // メモリ領域マッピング
    GetMapReslt mmapRslt = getMapedAddr(mMemDev, mapedBaseAddr, mapSize);

    // マッピング結果確認
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
/// 前回値保持書き込み
/// </summary>
/// <param name="setAddr">書き込み先アドレス</param>
/// <param name="setBitInf">マスク情報(書き込み対象bitを1とする)</param>
/// <param name="dataP">書き込みデータ(4byte)</param>
void MemMapCtrl::setDataHoldingPrevVal(unsigned int setAddr, const BitInf& setBitInf, const unsigned char dataP[4])
{

    unsigned char maskData[DEF_ACCESS_SIZE] = {0};     // マスクデータ
    // 書込み場所取得
    char* setAddrP = getOffsetMapedAdder(setAddr);

    // 前値取得
    uint32_t beforeData = *((uint32_t*)setAddrP);

    // マスクデータ生成(書込み対象bitのみを0としたデータ)
    createMaskData(maskData, setBitInf);

    // 書込み対象bitのみを0に落とした前値を生成し、
    // 生成した前値の書込み対象bitを書込みデータに変更する。
    //               書込み対象bitのみを0に落とした値 | 書込みデータ
    uint32_t writeData = ((beforeData & *((uint32_t*)maskData)) | *((uint32_t*)dataP));

    // マッピング領域へ書込み
    *((unsigned long *)setAddrP) = writeData;

    // レジスタへ同期
    msync(setAddrP, DEF_ACCESS_SIZE, MS_SYNC);
}


/// <summary>
/// データ書き込み
/// </summary>
/// <param name="setAddr">書き込み先アドレス</param>
/// <param name="size">書き込みサイズ</param>
/// <param name="dataP">書き込みデータ</param>
void MemMapCtrl::setMemData(unsigned int setAddr, int size , const unsigned char* dataP)
{
    // 書込み場所取得
    void* setAddrP = getOffsetMapedAdder(setAddr);
    // マッピング領域へ書込み
    memcpy(setAddrP, dataP, size);
    // レジスタへ同期
    msync(setAddrP, size, MS_SYNC);
}


/// <summary>
/// データ取得
/// </summary>
/// <param name="dataP">取得データ</param>
/// <param name="getAddr">取得先アドレス</param>
/// <param name="size">取得サイズ</param>
void MemMapCtrl::getMemData(unsigned char* dataP, unsigned int getAddr, int size)
{
    // 読み取り場所取得
    void* setAddrP = getOffsetMapedAdder(getAddr);
    // マッピング領域から取得
    memcpy(dataP, setAddrP, size);
}


/// <summary>
/// バイナリデータ→数値変換
/// 4byteのバイナリデータを数値に変換する
/// </summary>
/// <param name="bin">バイナリデータ</param>
/// <returns>数値データ</returns>
unsigned int MemMapCtrl::convBinToVal(unsigned char bin[4])
{
    unsigned int val = 0;

    // 1byteずつ数値の変換し、該当bitまでシフトさせる。
    unsigned int byte4 = 0;
    byte4 = bin[3];
    byte4 <<= 24;
    unsigned int byte3 = 0;
    byte3 = bin[2];
    byte3 <<= 16;
    unsigned int byte2 = 0;
    byte2 = bin[1];
    byte2 <<= 8;
    // 各byteを一つにまとめる
    val |= byte4;
    val |= byte3;
    val |= byte2;
    val |= bin[0];

    return val;
}

/// <summary>
/// バイナリデータ→数値変換
/// 2byteのバイナリデータを数値に変換する
/// </summary>
/// <param name="bin">バイナリデータ</param>
/// <returns>数値データ</returns>
unsigned short MemMapCtrl::convBinToVal2Byte(unsigned char bin[2])
{
    unsigned int val = 0;

    // 1byteずつ数値の変換し、該当bitまでシフトさせる。
    unsigned int byte2 = 0;
    byte2  = bin[1];
    byte2 <<= 8;
    // 各byteを一つにまとめる
    val |= byte2;
    val |= bin[0];

    return (unsigned short)val;
}


/// <summary>
/// 数値→バイナリデータ変換
/// 数値を4byteのバイナリデータに変換する
/// </summary>
/// <param name="bin">バイナリデータ</param>
/// <param name="val">数値</param>
void MemMapCtrl::convValToBin(unsigned char bin[4], unsigned int val)
{
    // 0クリア
    memset(bin, 0, DEF_ACCESS_SIZE);

    // 1byteずつ順に取り出して書き込む
    bin[3] = (unsigned char)((val & 0xFF000000) >> 24);
    bin[2] = (unsigned char)((val & 0x00FF0000) >> 16);
    bin[1] = (unsigned char)((val & 0x0000FF00) >> 8);
    bin[0] = (unsigned char)(val & 0x000000FF);
}


/// <summary>
/// メモリマッピング領域取得
/// </summary>
/// <param name="aligned">アライメント調整後のサイズ</param>
/// <param name="unmap_addr">mmapの戻り値のアドレス</param>
/// <param name="mem_device">メモリデバイス</param>
/// <param name="addr">マッピングを行う先頭アドレス</param>
/// <param name="size">マッピングサイズ</param>
/// <returns>マッピング結果</returns>
GetMapReslt MemMapCtrl::getMapedAddr(int mem_device, uint32_t addr, uint32_t size)
{
    GetMapReslt rslt = {};
    void* mem;
    char* aligned_vaddr;
    uint32_t aligned_paddr;
    uint32_t aligned_size;

    // 4byte境界に合わせる（4未満ならより手前の境界に、5以上8未満ならより後ろの境界に合わせる）
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

    // マッピング
    aligned_vaddr = (char*)mmap(NULL, aligned_size, (PROT_WRITE | PROT_READ), MAP_SHARED, mem_device, aligned_paddr);
    if (aligned_vaddr == MAP_FAILED) {
        printf("Error: get_maped_addr: Error mapping address\n");
        return nullptr;
    }
    // オフセット調整
    mem = (void *)(aligned_vaddr + (paddr - aligned_paddr));
    
    // 戻り値設定
    rslt.aligned = aligned_size;
    rslt.unmapAddr = aligned_vaddr;
    rslt.mmapAddr = mem;
}


//--------------------------------------------------------------------
//    メソッド名 : getOffsetMapedAdder
//      日本語名 :
//        概  要 : 指定アドレスに対応したマッピング領域アドレスを取得する
//    パラメータ : [IN] addr : 取得したいアドレス
//      復帰情報 : マッピング領域アドレス
//        備  考 : なし
//      更新履歴 :
//        日  付   内  容
//    2018/mm/dd   新規作成
//--------------------------------------------------------------------
char* MemMapCtrl::getOffsetMapedAdder(unsigned int addr)
{
    return (char*)(mMemMappedRegionP + (addr - mMemMapBaseAddr));
}



//--------------------------------------------------------------------
//    メソッド名 : createMaskData
//      日本語名 : マスクデータ生成
//        概  要 : 書込み時のマスクデータを生成する。
//    パラメータ : [OUT]  maskData  マスクデータ
//               : [IN ] bitInf    マスク用のbit情報
//      復帰情報 :
//        備  考 : なし
//      更新履歴 :
//        日  付   内  容
//    2018/mm/dd   新規作成
//--------------------------------------------------------------------
void MemMapCtrl::createMaskData(unsigned char maskData[4], const  BitInf& bitInf)
{
    // 各bitを反転させ、書込み対象bit以外を1としたマスクデータを生成する
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