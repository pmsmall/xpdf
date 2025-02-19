//========================================================================
//
// String.cc
//
// Variable-length string type.
//
//========================================================================

#pragma implementation

#include <stdlib.h>
#include <string.h>
#include <String.h>

static inline int size(int len) {
  return ((len + 1) + 7) & ~7;
}

inline void String::resize(int length1) {
  char *s1;

  if (!s) {
    s = new char[size(length1)];
  } else if (size(length1) != size(length)) {
    s1 = new char[size(length1)];
    strncpy(s1, s, length1);
    delete s;
    s = s1;
  }
}

String::String() {
  s = NULL;
  resize(length = 0);
  s[0] = '\0';
}

String::String(char *s1) {
  s = NULL;
  resize(length = (int)strlen(s1));
  strcpy(s, s1);
}

String::String(char *s1, int length1) {
  s = NULL;
  resize(length = length1);
  strncpy(s, s1, length);
  s[length] = '\0';
}

String::String(String *str) {
  s = NULL;
  resize(length = str->getLength());
  strcpy(s, str->getCString());
}

String::String(String *str1, String *str2) {
  s = NULL;
  resize(length = str1->getLength() + str2->getLength());
  strcpy(s, str1->getCString());
  strcpy(s + str1->getLength(), str2->getCString());
}

String::~String() {
  delete s;
}

String *String::clear() {
  resize(0);
  s[length = 0] = '\0';
  return this;
}

String *String::append(char c) {
  resize(length + 1);
  s[length++] = c;
  s[length] = '\0';
  return this;
}

String *String::append(String *str) {
  resize(length + str->getLength());
  strcpy(s + length, str->getCString());
  length += str->getLength();
  return this;
}

String *String::append(char *str) {
  int n = strlen(str);

  resize(length + n);
  strcpy(s + length, str);
  length += n;
  return this;
}

String *String::insert(int i, char c) {
  int j;

  resize(length + 1);
  for (j = length + 1; j > i; --j)
    s[j] = s[j-1];
  s[i] = c;
  ++length;
  return this;
}

String *String::insert(int i, String *str) {
  int n = str->getLength();
  int j;

  resize(length + n);
  for (j = length; j >= i; --j)
    s[j+n] = s[j];
  strncpy(s+i, str->getCString(), n);
  length += n;
  return this;
}

String *String::insert(int i, char *str) {
  int n = strlen(str);
  int j;

  resize(length + n);
  for (j = length; j >= i; --j)
    s[j+n] = s[j];
  strncpy(s+i, str, n);
  length += n;
  return this;
}

String *String::del(int i, int n) {
  int j;

  if (n > 0) {
    for (j = i; j <= length - n; ++j)
      s[j] = s[j + n];
    resize(length -= n);
  }
  return this;
}
