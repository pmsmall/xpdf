//========================================================================
//
// String.h
//
// Variable-length string type.
//
//========================================================================

#ifndef VSTRING_H
#define VSTRING_H

#pragma interface

class String {
public:

  // Create an empty string.
  String();

  // Create a string from a C string.
  String(char *s1);

  // Create a string from the first <length1> chars of a C string.
  String (char *s1, int length1);

  // Copy a string.
  String(String *str);
  String *copy() { return new String(this); }

  // Concatenate two strings.
  String(String *str1, String *str2);

  // Destructor.
  ~String();

  // Get length.
  int getLength() { return length; }

  // Get C string.
  char *getCString() { return s; }

  // Get <i>th character.
  char getChar(int i) { return s[i]; }

  // Clear string to zero length.
  String *clear();

  // Append a character or string.
  String *append(char c);
  String *append(String *str);
  String *append(char *str);

  // Insert a character or string.
  String *insert(int i, char c);
  String *insert(int i, String *str);
  String *insert(int i, char *str);

  // Delete a character or range of characters.
  String *del(int i, int n = 1);

  // Compare two strings:  -1:<  0:=  +1:>
  int cmp(String *str) { return strcmp(s, str->getCString()); }
  int cmp(char *s1) { return strcmp(s, s1); }

private:

  int length;
  char *s;

  void resize(int length1);
};

#endif
