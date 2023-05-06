#include <stdlib.h>
#include <stdint.h>

void base64_testunit1();




void base64_encode(uint8_t const *src, size_t srclen, char *dst, size_t dstlen);
void base64_decode(char const *src, size_t srclen, uint8_t *dst, size_t dstlen);


size_t base64_decoded_len(const char *data, size_t len);
size_t base64_encoded_len(size_t len);