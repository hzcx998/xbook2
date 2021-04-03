#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// the max capacity of one string
#define MAX_ARRAY_CAPACITY 999999

// Pattern Definitions
char pattern[MAX_ARRAY_CAPACITY];
// KMP Next Array
int next_array[MAX_ARRAY_CAPACITY];

// get next KMP array
void getNextKMPArray() {
  unsigned long pattern_size = strlen(pattern + 1);
  for(int i = 2, j = 0; i <= pattern_size; i++) {
    while(j && pattern[i] != pattern[j + 1]) j = next_array[j];
    if(pattern[i] == pattern[j + 1]) j++;
    next_array[i] = j;
  }
}

// KMP Match just judge
int judgeContains(char* str) {
  unsigned long str_size = strlen(str + 1);
  unsigned long pattern_size = strlen(pattern + 1);
  for(int i = 1, j = 0; i <= str_size; i++) {
    while(j && str[i] != pattern[j + 1]) j = next_array[j];
    if(str[i] == pattern[j + 1]) j++;
    if(j == pattern_size) return 1;
  }
  return 0;
}

char* readFile(const char* filepath) {
  int fd = open(filepath, O_RDONLY);
  if(fd == -1) {
    printf("grep:file %s not exist\n", filepath);
    return NULL;
  }
  struct stat fstat;
  stat(filepath, &fstat);

  char *buf = (char *)malloc(fstat.st_size);
  read(fd, buf, fstat.st_size);
  close(fd);
  return buf;
}

void handleOneFile(char* filename) {
  char* content = readFile(filename);
  int i = 0, j = 1;
  char line[MAX_ARRAY_CAPACITY];
  while(1) {
    line[j++] = content[i++];
    if(content[i] == '\n' || content[i] == 0) {
      line[j] = 0; j = 1;
      // handle line
      if(judgeContains(line)) printf("%s\n", line + 1);
      while(content[i] == '\n') i++;
      if(content[i] == 0) break;
    }
  }
}

void handleOneFileAndPrintFileName(char* filename) {
  char* content = readFile(filename);
  int i = 0, j = 1;
  char line[MAX_ARRAY_CAPACITY];
  while(1) {
    line[j++] = content[i++];
    if(content[i] == '\n' || content[i] == 0) {
      line[j] = 0; j = 1; 
      // handle line
      if(judgeContains(line)) printf("%s:%s\n", filename, line + 1);
      while(content[i] == '\n') i++;
      if(content[i] == 0) break;
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc < 3) {
    printf("Usage: grep PATTERN [FILE]...\n");
    return -1;
  }

  // strcpy pattern from index 1
  strcpy(pattern + 1, argv[1]);
  // get next KMP Array
  getNextKMPArray();

  if(argc == 3) handleOneFile(argv[2]);
  else {
    for(int i = 2; i < argc; i++) 
      handleOneFileAndPrintFileName(argv[i]);
  }
  return 0;
}

