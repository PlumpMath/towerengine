#ifndef _TOWERENGINE_UTILS_H
#define _TOWERENGINE_UTILS_H

#define degtorad(x) (x * M_PI / 180.0)

#define safe_delete(x) { delete x; x = 0; }

char *ftoa(float f, int len = 50);
char *itoa(int v, int len = 50);
std::string itos(int v);
char *cstr(const char *);

std::string PathOfFile(std::string file);


int Base64Decode(char *in, size_t inLen, unsigned char *out, size_t *outLen);
void Base64Decode(char *in, unsigned char **out, size_t *out_size);

float RandomFloat(float min = 0.0, float max = 1.0);
int RandomInt(int min, int max);
float Mix(float a, float b, float mix);

char *ReadFile(const char *file); // zero-terminated
char *ReadFile(const char *file, size_t &size); // not zero-terminated

#endif
