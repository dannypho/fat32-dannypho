#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5

FILE *fp;
int open = 0;

char BS_OEMName[8];
int16_t BPB_BytsPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATs;
int16_t BPB_RootEntCnt;
char BS_VolLab[11];
int32_t BPB_FATSz32;
int32_t BPB_ExtFlags;
int32_t BPB_RootClus;
int32_t BPB_FSInfo;

int32_t RootDirSectors = 0;
int32_t FirstDataSector = 0;
int32_t FirstSectorofCluster = 0;

struct __attribute__((__packed__)) DirectoryEntry {
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];

int LBAtoOffset(int32_t sector)
{
  return ((sector - 2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytsPerSec);
}

char *convert_to_fat_name(char *input)
{
  char *expanded_name = malloc(12 * sizeof(char));
  memset( expanded_name, ' ', 12 );

  char *token = strtok( input, "." );

  strncpy( expanded_name, token, strlen( token ) );

  token = strtok( NULL, "." );

  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }

  expanded_name[11] = '\0';

  int i;
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }
  return expanded_name;
}

void open_fat(char *filename)
{
  fp = fopen(filename, "r+");
  if ( fp == NULL )
  {
    printf("Error: File system image not found.\n");
  }
  else if( open == 1 )
  {
    printf("Error: File system image already open.\n");
  }
  else
  {
    fseek(fp, 11, SEEK_SET);
    fread(&BPB_BytsPerSec, 2, 1, fp);

    fseek(fp, 13, SEEK_SET);
    fread(&BPB_SecPerClus, 1, 1, fp);

    fseek(fp, 14, SEEK_SET);
    fread(&BPB_RsvdSecCnt, 2, 1, fp);

    fseek(fp, 16, SEEK_SET);
    fread(&BPB_NumFATs, 1, 1, fp);

    fseek(fp, 36, SEEK_SET);
    fread(&BPB_FATSz32, 4, 1, fp);

    fseek(fp, 40, SEEK_SET);
    fread(&BPB_ExtFlags, 2, 1, fp);

    fseek(fp, 44, SEEK_SET);
    fread(&BPB_RootClus, 4, 1, fp);

    fseek(fp, 48, SEEK_SET);
    fread(&BPB_FSInfo, 2, 1, fp);

    // Calculate root directory and populate DirectoryEntry struct
    int32_t offset = (BPB_BytsPerSec * BPB_RsvdSecCnt) + (BPB_BytsPerSec * BPB_FATSz32 * BPB_NumFATs);
    fseek(fp, offset, SEEK_SET);
    fread(&dir[0], sizeof(struct DirectoryEntry) * 16, 1, fp);

    open = 1;
  }
}

void info()
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  printf("BPB_BytesPerSec \t%d\t0x%x\n", BPB_BytsPerSec, BPB_BytsPerSec);
  printf("BPB_SecPerClus \t\t%d\t0x%x\n", BPB_SecPerClus, BPB_SecPerClus);
  printf("BPB_RsvdSecCnt \t\t%d\t0x%x\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
  printf("BPB_NumFATs \t\t%d\t0x%x\n", BPB_NumFATs, BPB_NumFATs);
  printf("BPB_FATSz32 \t\t%d\t0x%x\n", BPB_FATSz32, BPB_FATSz32);
  printf("BPB_ExtFlags \t\t%d\t0x%x\n", BPB_ExtFlags, BPB_ExtFlags);
  printf("BPB_RootClus \t\t%d\t0x%x\n", BPB_RootClus, BPB_RootClus);
  printf("BPB_FSInfo \t\t%d\t0x%x\n", BPB_FSInfo, BPB_FSInfo);
}

void close_fat()
{
  if ( open == 1 )
  {
    fclose(fp);
    open = 0;
  }
  else
  {
    printf("Error: File system not open.\n");
  }
}

void ls()
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  for( int i = 0; i < 16; i ++)
  { 
    if ( (dir[i].DIR_Attr == 1 || dir[i].DIR_Attr == 16 || dir[i].DIR_Attr == 32) && (dir[i].DIR_Name[0] != -27) )
    {
      char name[12];
      strncpy(name, dir[i].DIR_Name, 11);
      name[11] = '\0';
      printf("%s\n", name);
    }
  }
}

void stat(char *filename)
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  for( int i = 0; i < 16; i ++)
  { 
    char name[12];
    strncpy(name, dir[i].DIR_Name, 11);
    name[11] = '\0';
    if ( strcmp(name, filename) == 0 )
    {
      printf("\nDIR_ATTR \t\t%d\n", dir[i].DIR_Attr);
      printf("Unused1[8] \t\t");
      int length = sizeof(dir[i].Unused1) / sizeof(dir[i].Unused1[0]);
      for (int j = 0; j < length; j++)
      {
        printf("%d ", dir[i].Unused1[j]);
      }
      printf("\n");
      printf("DIR_FirstClusterHigh \t%d\n", dir[i].DIR_FirstClusterHigh);
      printf("Unused2[4] \t\t");
      length = sizeof(dir[i].Unused2) / sizeof(dir[i].Unused2[0]);
      for (int j = 0; j < length; j++)
      {
        printf("%d ", dir[i].Unused2[j]);
      }
      printf("\n");
      printf("DIR_FirstClusterLow \t%d\n", dir[i].DIR_FirstClusterLow);
      printf("DIR_FileSize \t\t%d\n\n", dir[i].DIR_FileSize);
    }
  }
}

void delete(char *filename)
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  for ( int i = 0; i < 16; i++ )
  {
    char name[11];
    strncpy(name, dir[i].DIR_Name, 11);
    if ( strcmp(name, filename) == 0)
    {
      dir[i].DIR_Name[0] = 0xe5;
    }
  }
}

void undelete(char *filename)
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  char deleted_file[12];
  strncpy(deleted_file, filename, 11);
  deleted_file[11] = '\0';
  deleted_file[0] = 0xe5;

  for ( int i = 0; i < 16; i++ )
  {
    char name[12];
    strncpy(name, dir[i].DIR_Name, 11);
    name[11] = '\0';
    if ( strcmp(deleted_file, name) == 0)
    { 
      dir[i].DIR_Name[0] = filename[0];
    }
  }

}

void cd(char *directoryname)
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  for ( int i = 0; i < 16; i++ )
  {
    char name[12];
    strncpy(name, dir[i].DIR_Name, 11);
    name[11] = '\0';
    // Check if the string passed after 'cd' (directoryname) matches an entry (name) in the directory struct
    if ( strcmp(directoryname, name) == 0)
    { 
      // Calculate cluster number and offset of the directory to go to
      int32_t cluster_number = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
      int32_t offset = LBAtoOffset(cluster_number);
      fseek(fp, offset, SEEK_SET);
      fread(&dir[0], sizeof(struct DirectoryEntry) * 16, 1, fp);
      return;
    }
  }

}

void read(char *filename, int position, int number_of_bytes, char *option)
{
  if ( open == 0 )
  {
    printf("Error: File system image must be opened first\n");
    return;
  }
  for ( int i = 0; i < 16; i++ )
  {
    char name[12];
    strncpy(name, dir[i].DIR_Name, 11);
    name[11] = '\0';
    // Check if the string passed after 'read' (directoryname) matches an entry (name) in the directory struct
    if ( strcmp(filename, name) == 0)
    { 
      int32_t cluster_number = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
      int32_t offset = LBAtoOffset(cluster_number);
      int32_t start_position = offset + position;
      int32_t end_position = offset + position + number_of_bytes;
      int32_t length = end_position - start_position;
      uint8_t value;
      for (int j = 0; j < length; j++)
      { 
        fseek(fp, start_position + j, SEEK_SET);
        fread(&value, 1, 1, fp);
        if (strcmp(option, "-ascii") == 0)
        {
          printf("%c ", value);
        }
        else if (strcmp(option, "-dec") == 0)
        {
          printf("%d ", value);
        }
        else
        {
          printf("%x ", value);
        }
      }
      printf("\n");
    }
  }
}

int main()
{
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
  while( 1 )
  {
    printf ("mfs> ");

    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );
    char *token[MAX_NUM_ARGUMENTS];
    int token_count = 0;                                 
                                                           

    char *argument_pointer;                                         
                                                           
    char *working_string  = strdup( command_string );                

    char *head_ptr = working_string;
    
    while ( ( (argument_pointer = strsep(&working_string, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_pointer, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
   
    if ( strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0 )
    {
      exit(0);
    }
    if ( strcmp(token[0], "open") == 0 )
    {
      open_fat(token[1]);
    }
    if ( strcmp(token[0], "close") == 0 )
    {
      close_fat();
    }
    if ( strcmp(token[0], "info") == 0 )
    {
      info();
    }
    if ( strcmp(token[0], "ls") == 0 )
    {
      ls();
    }
    if ( strcmp(token[0], "stat") == 0)
    {
      if ( token[1] != NULL)
      { 
        char *input = convert_to_fat_name(token[1]);
        stat(input);
      }
    }
    if ( strcmp(token[0], "del") == 0 )
    {
      if ( token[1] != NULL)
      {
        char *input = convert_to_fat_name(token[1]);
        delete(input);
      }
    }
    if ( strcmp(token[0], "undel") == 0 )
    {
      if ( token[1] != NULL)
      {
        char *input = convert_to_fat_name(token[1]);
        undelete(input);
      }
    }
    if ( strcmp(token[0], "cd") == 0 )
    { 
      if ( token[1] != NULL)
      {
        if ( strcmp(token[1], "..") == 0)
        {
          cd("..         ");
        }
        else
        { 
          char *input = convert_to_fat_name(token[1]);
          cd(input);
        }
      }
    }
    if (strcmp(token[0], "read") == 0)
    {
      if(token[1] != NULL && token[2] != NULL && token[3] != NULL && token[4] == NULL)
      {
        char *input = convert_to_fat_name(token[1]);
        int position = atoi(token[2]);
        int number_of_bytes = atoi(token[3]);
        read(input, position, number_of_bytes, NULL);
      }
      if(token[1] != NULL && token[2] != NULL && token[3] != NULL && token[4] != NULL)
      {
        char *input = convert_to_fat_name(token[1]);
        int position = atoi(token[2]);
        int number_of_bytes = atoi(token[3]);
        read(input, position, number_of_bytes, token[4]);
      }
    }

    free( head_ptr );
  }
  return 0;
}