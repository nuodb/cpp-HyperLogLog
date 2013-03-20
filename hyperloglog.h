#ifndef HYPERLOGLOG_H
#define HTPERLOGLOG_H

#include<vector>
#include<cmath>
#include"murmur3.h"

#define HLL_HASH_SEED 313

static const double pow_2_32 =  4294967296.0;
static const double neg_pow_2_32 = -4294967296.0;

class HyperLogLog {
private:
    std::vector<uint8_t> M;
    uint8_t  b;
    uint32_t m;
    double alphaMM;

    uint8_t rho(uint32_t x, uint8_t b) {
        uint8_t v = 1;
        while (v <= b && !(x & 0x80000000)) {
            v++;
            x <<= 1;
        }
        return v;
    }

public:

    HyperLogLog(uint8_t b_) : b(b_), m(1<<b), M(m+1,0) {
       std::cout << m << std::endl; 

       double alpha = 0.0;
       switch(m){
           case 16:
               alpha = 0.673;
               break;
           case 32:
               alpha = 0.697;
               break;
           case 64:
               alpha = 0.709;
               break;
           default:
               alpha = 0.7213/(1.0 + 1.079/m);
       }
       alphaMM = alpha * m * m;

    }

    void add(const char* str, uint32_t len){
        uint32_t hash;
        MurmurHash3_x86_32(str,len,HLL_HASH_SEED, (void*)&hash);
        uint32_t index = hash >> ( 32 - b );
        uint8_t rank = rho((hash << b), 32 - b);
        if( rank > M[index] ){
            M[index] = rank;
        }
    }

    double estimate(){
        double estimate;
        uint8_t rank = 0;
        double sum = 0.0;
        for (uint32_t i = 0; i < m; i++) {
            sum += 1.0/pow(2.0, M[i]);
        }
        estimate = alphaMM/sum; // E in the original paper
        if( estimate <= 2.5 * m ) {
            uint32_t zeros = 0;
            for (uint32_t i = 0; i < m; i++) {
                if (M[i] == 0) {
                    zeros++;
                }
            }
            if( zeros != 0 ) {
                estimate = m * log((double)m/zeros);
            }
        } else if (estimate > (1.0/30.0) * pow_2_32) {
            estimate = neg_pow_2_32 * log(1.0 - ( estimate/pow_2_32 ) );
        }
    }

};

#endif

