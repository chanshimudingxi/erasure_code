/*
 * (C) 2007-2010 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version: $Id: erasure_code.cpp 706 2012-07-19 15:30:41Z linqing.zyd@taobao.com $
 *
 * Authors:
 *   linqing <linqing.zyd@taobao.com>
 *      - initial release
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "galois.h"
#include "jerasure.h"
#include "erasure_code.h"

const int ErasureCode::ws_ = 8;
const int ErasureCode::ps_ = 1024;
std::mutex ErasureCode::mutex_;

ErasureCode::ErasureCode()
{
  matrix_ = NULL;
  de_matrix_ = NULL;
  clear();
}

ErasureCode::~ErasureCode()
{
  free(matrix_);
  free(de_matrix_);
}

int ErasureCode::config(const int dn, const int pn, int* erased)
{
  int ret = 0;

  clear();

  dn_ = dn;
  pn_ = pn;

  matrix_ = (int*)malloc(dn_*pn_ * sizeof(int32_t));
  assert(NULL != matrix_);
  mutex_.lock();
  for (int i = 0; i < pn_; i++)
  {
    for (int j = 0; j < dn_; j++)
    {
      matrix_[i*dn_+j] = galois_single_divide(1, i ^ (pn_ + j), ws_);
    }
  }
  mutex_.unlock();
  int* bitmatrix = jerasure_matrix_to_bitmatrix(dn_, pn_, ws_, matrix_);
  assert(NULL != bitmatrix);
  free(matrix_);
  matrix_ = bitmatrix;

  // if need decode, cache decode matrix
  // compute decode matrix is a costful work
  if (NULL != erased)
  {
    de_matrix_ = (int*)malloc(dn_*dn_*ws_*ws_*sizeof(int32_t));
    assert(NULL != de_matrix_);
    int alive = 0;
    for (int i = 0; i < dn_ + pn_; i++)
    {
      erased_[i] = erased[i];
      if (0 == erased[i])
      {
        alive++;
      }
    }

    if (alive < dn_)
    {
      ret = -1;
      fprintf(stderr, "no enough alive data for decode, alive: %d, ret: %d", alive, ret);
    }
    else
    {
      if (jerasure_make_decoding_bitmatrix(dn_, pn_, ws_,
            matrix_, erased, de_matrix_, dm_ids_) < 0)
      {
        ret = -1;
        fprintf(stderr, "can't make decoding bitmatrix, ret: %d", ret);
      }
    }
  }

  return ret;
}

void ErasureCode::bind(char* data, const int index, const int size)
{
  data_[index] = data;
  size_[index] = size;
}

void ErasureCode::bind(char** data, const int len, const int size)
{
  if (NULL != data)
  {
    int n = len < dn_ + pn_ ? len: dn_ + pn_;
    for (int i = 0; i < n; i++)
    {
      bind(data[i], i, size);
    }
  }
}

char* ErasureCode::get_data(const int index)
{
  return data_[index];
}

void ErasureCode::clear()
{
  dn_ = -1;
  pn_ = -1;
  free(matrix_);
  free(de_matrix_);
  for (int i = 0; i < MAX_MARSHALLING_NUM; i++)
  {
    data_[i] = NULL;
    size_[i] = -1;
    dm_ids_[i] = -1;
    erased_[i] = -1;
  }
}

int ErasureCode::encode(const int size)
{
  int ret = 0;

  if (NULL == matrix_)
  {
    ret = -1;
    fprintf(stderr, "matrix invalid, call config() first.");
  }
  else if (0 != size % (ws_ * ps_))
  {
    ret = -1;
    fprintf(stderr, "size(%d) %% (ws*ps) != 0, invalid.", size);
  }
  else
  {
    for (int i = 0; i < dn_ + pn_; i++)
    {
      if (NULL == data_[i] || size_[i] < size)
      {
        fprintf(stderr, "data_[%d] %p size %d esize %d", i, data_[i], size_[i], size);
        ret = -1;
        break;
      }
    }
  }

  if (0 == ret)
  {
    jerasure_bitmatrix_encode(dn_, pn_, ws_, matrix_, data_, data_ + dn_, size, ps_);
  }

  return ret;
}

int ErasureCode::decode(const int size)
{
  int ret = 0;

  if (NULL == de_matrix_)
  {
    ret = -1;
    fprintf(stderr, "matrix invalid, call config() first");
  }
  else if (0 != size % (ws_ * ps_))
  {
    ret = -1;
    fprintf(stderr, "size(%d) %% (ws*ps) != 0, invalid.", size);
  }
  else
  {
    for (int i = 0; i < dn_ + pn_; i++)
    {
      if (NULL == data_[i] || size_[i] < size)
      {
        fprintf(stderr, "data_[%d] %p size %d esize %d", i, data_[i], size_[i], size);
        ret = -1;
        break;
      }
    }
  }

  if (0 == ret)
  {
    // recovery data
    for (int i = 0; i < dn_; i++)
    {
      if (erased_[i])
      {
        jerasure_bitmatrix_dotprod(dn_, ws_, de_matrix_ + i * dn_ * ws_ * ws_,
            dm_ids_, i, data_, data_ + dn_, size, ps_);
      }
    }

    // recovery pairty
    for (int i = 0; i < pn_; i++)
    {
      if (1 == erased_[dn_+i])
      {
        jerasure_bitmatrix_dotprod(dn_, ws_, matrix_ + i * dn_ * ws_ * ws_,
            NULL, dn_ + i, data_, data_ + dn_, size, ps_);
      }
    }
  }

  return ret;
}
