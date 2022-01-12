#pragma once

#include<iostream>
#include<math.h>
using namespace std;

template <typename T>
struct Signature {
    virtual T generatePublicKey() = 0;
    virtual T generatePrivateKey() = 0;
    virtual T sign(T msg, T privateKey) = 0;
    virtual T verify(T msg, T publicKey) = 0;
};

struct RsaSignature : Signature<double> {
    int p;
    int q;
    int phi;
    int n;
    int e;
    double d;
    RsaSignature () {}
    RsaSignature(int p, int q)
        : p(p), q(q) {
        this->n = p * q;
        this->phi = (p - 1) * (q - 1);
    }

    int gcd(int a, int b) {
        int t;
        while(1) {
            t= a%b;
            if(t==0)
                return b;
            a = b;
            b= t;
        }
    }


    double generatePublicKey() {
        e = 2; // 1 < e < phi(n)
        double tmp = 0;
        while (e < phi) {
            if (gcd(e, phi) == 1) {
                tmp = e;
            }
            e++;
        }
        e = tmp;
        return e;
    }

    double generatePrivateKey() {
        double tmp = 1. / this->e;
        d = fmod(tmp, phi);
        return d;
    }

    double sign(double msg, double privateKey) {
        double encrypted = pow(msg, privateKey);
        return encrypted;
    }

    double verify(double msg, double publicKey) {
        double decrypted = pow(msg, publicKey);
        return decrypted;
    }


};


