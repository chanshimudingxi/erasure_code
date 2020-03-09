/*
 * (C) 2007-2010 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version: $Id: erasure_code.h 706 2012-07-19 15:30:41Z linqing.zyd@taobao.com $
 *
 * Authors:
 *   linqing <linqing.zyd@taobao.com>
 *      - initial release
 */


#ifndef ERASURE_CODE_H_
#define ERASURE_CODE_H_

#include <mutex>

//编组方案限定，放宽到36+18
#define MAX_MARSHALLING_NUM 54

class ErasureCode
{
  public:
    /**
     * @brief constructor
     */
    ErasureCode();

    /**
     * @brief destructor
     */
    virtual ~ErasureCode();

    /**
    * @brief config erasure code
    *
    * @param dn: data numbers
    * @param pn: parity numbers
    * @param erased: which disk is erased
    *        only used on decode, length: dn + pn
    *        alive: 0, dead: 1, not used: -1
    * @return 0 on succss
    */
    int config(const int dn, const int pn, int* erased = NULL);

    /**
     * @brief bind data with encode/decode buffer
     *
     * @param data: data buffer
     * @param index: the data index
     * @param size: data size
     */
    void bind(char* data, const int index, const int size);

    /**
    * @brief return the data buffer by specified index
    *
    * @return
    */
    char* get_data(const int index);

    /**
     * @brief bind data
     *
     * @param data: data pointer array
     * @param len: pointer array size
     * @param size: data size
     */
    void bind(char** data, const int len, const int size);

    /**
     * @brief clear
     */
    void clear();

    /**
     * @brief encode data specified by size
     * should implemented by specific algorithm
     *
     * @param size: size to encode
     *
     * @return 0 on success
     */
    int encode(const int size);

    /**
     * @brief decode data specified by size
     *
     * should implemented by specific algorithm
     *
     * @param size
     *
     * @return
     */
    int decode(const int size);

  public:
    static const int ws_;         // word size, shouldn't change
    static const int ps_;         // packet_size, shouldn't change

  public:
    enum
    {
      NODE_ALIVE = 0,
      NODE_DEAD = 1,
      NODE_UNUSED = -1
    };

  private:
    ErasureCode& operator = (const ErasureCode&) = delete;
    int dn_;                                      // data numbers
    int pn_;                                      // parity numbers
    int* matrix_;                                 // encode matrix
    int* de_matrix_;                              // decode matrix
    char* data_[MAX_MARSHALLING_NUM];             // data binding
    int size_[MAX_MARSHALLING_NUM];               // size of each stripe
    int dm_ids_[MAX_MARSHALLING_NUM];             // alive disks
    int erased_[MAX_MARSHALLING_NUM];             // dead disk byte map

    static std::mutex mutex_;                  // muext for create matrix
};
#endif
