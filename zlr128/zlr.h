#ifndef ZLR_H
#define ZLR_H
void dszlr_encrypt(const unsigned char *ass_data, unsigned long long int ass_data_len,
                   const unsigned char *message, unsigned long long int m_len,
                   const unsigned char *key, unsigned char *ciphertext);
#endif /* ZLR_H */
