#include "XmlReader.h"
#include <expat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void startXml(void* userData, const char* name, const char** atts);
void endXml(void* userData, const char* name);
void valueXml(void* userData, const char* val, int len);
void copyString(char** destination, const char* source);
FileInfo* fileById(const char* id, FileList* list);

typedef struct ExpatUserData
{
    FileList* fileList;
    AlbumList* albumList;
    FileInfo* currentFile;
    Album* currentAlbum;
    char valueBuffer[BUFFER_SIZE + 1];
    unsigned int currentIndex;
} ExpatUserData;

int
readPlaylistFile(
    const char* filePath,
    FileList* fileList,
    AlbumList* albumList)
{
    int result = 0;
    XML_Parser parser = XML_ParserCreate(NULL);
    FILE *fileHandle = fopen(filePath, "r");
    ExpatUserData userData;
    char buffer[BUFFER_SIZE];
    unsigned int len;

    if (parser == 0) {
        result = -2;
    } else if (fileHandle != 0) {
        XML_SetElementHandler(parser, startXml, endXml);
        XML_SetCharacterDataHandler(parser, valueXml);

        userData.fileList = fileList;
        userData.albumList = albumList;
        userData.currentFile = 0;
        userData.currentAlbum = 0;
        userData.valueBuffer[BUFFER_SIZE] = '\0';
        XML_SetUserData(parser, (void*) &userData);

        len = fread(buffer, sizeof(char), sizeof(buffer), fileHandle);
        while (len > 0) {
            if (XML_Parse(parser, buffer, len, len < BUFFER_SIZE) == 0) {
                result = XML_GetErrorCode(parser);
                break;
            }

            len = fread(buffer, sizeof(char), sizeof(buffer), fileHandle);
        }

        fclose(fileHandle);
        XML_ParserFree(parser);
    } else {
        result = -1;
    }

    return result;
}

void
startXml(void* userData, const char* name, const char** atts)
{
    ExpatUserData *data = (ExpatUserData*) userData;
    unsigned int index = 0;
    FileInfo *fileInfoPtr;
    Album *albumPtr;

    if (strcmp(name, "FileList") == 0) {
        data->fileList->count = 0;
        data->fileList->list = 0;

        while (atts[index] != 0) {
            if (strcmp(atts[index], "count") == 0) {
                ++index;
                data->fileList->count = atoi(atts[index]);
            }
            ++index;
        }

        if (data->fileList->count > 0) {
            data->fileList->list =
                (FileInfo*) malloc(data->fileList->count * sizeof(FileInfo));
            fileInfoPtr = data->fileList->list;
            data->currentFile = fileInfoPtr;

            index = 0;
            while (index < data->fileList->count) {
                fileInfoPtr->id = 0;
                fileInfoPtr->filePath = 0;
                fileInfoPtr->title = 0;
                fileInfoPtr->album = 0;
                fileInfoPtr->artist = 0;
                fileInfoPtr->genre = 0;
                fileInfoPtr->comment = 0;
                fileInfoPtr->albumArt = 0;
                fileInfoPtr->year = 0;
                fileInfoPtr->favorite = 0;

                ++fileInfoPtr;
                ++index;
            }
        }
    } else if (strcmp(name, "AlbumList") == 0) {
        data->albumList->count = 0;
        data->albumList->list = 0;

        while (atts[index] != 0) {
            if (strcmp(atts[index], "count") == 0) {
                ++index;
                data->albumList->count = atoi(atts[index]);
            }
            ++index;
        }

        if (data->albumList->count > 0) {
            data->albumList->list =
                (Album*) malloc(data->albumList->count * sizeof(Album));
            albumPtr = data->albumList->list;
            data->currentAlbum = albumPtr;

            index = 0;
            while (index < data->albumList->count) {
                albumPtr->id = 0;
                albumPtr->name = 0;
                albumPtr->algorithm = 0;
                albumPtr->songs = 0;
                albumPtr->albumArt = 0;
                albumPtr->count = 0;
                albumPtr->shuffle = 0;
                albumPtr->repeat = 0;

                ++albumPtr;
                ++index;
            }
        }
    } else if (strcmp(name, "file") == 0 && data->currentFile != 0) {
        while (atts[index] != 0) {
            if (strcmp(atts[index], "id") == 0) {
                ++index;
                copyString(&data->currentFile->id, atts[index]);
            } else if (strcmp(atts[index], "favorite") == 0) {
                ++index;
                if (strcmp(atts[index], "1") == 0) {
                    data->currentFile->favorite = 1;
                } else {
                    data->currentFile->favorite = 0;
                }
            }
            ++index;
        }
    } else if (strcmp(name, "Album") == 0 && data->currentAlbum != 0) {
        while (atts[index] != 0) {
            if (strcmp(atts[index], "id") == 0) {
                ++index;
                copyString(&data->currentAlbum->id, atts[index]);
            } else if (strcmp(atts[index], "name") == 0) {
                ++index;
                copyString(&data->currentAlbum->name, atts[index]);
            } else if (strcmp(atts[index], "shuffle") == 0) {
                ++index;
                if (strcmp(atts[index], "1") == 0) {
                    data->currentAlbum->shuffle = 1;
                } else {
                    data->currentAlbum->shuffle = 0;
                }
            } else if (strcmp(atts[index], "repeat") == 0) {
                ++index;
                if (strcmp(atts[index], "1") == 0) {
                    data->currentAlbum->repeat = 1;
                } else {
                    data->currentAlbum->repeat = 0;
                }
            } else if (strcmp(atts[index], "count") == 0) {
                ++index;
                data->currentIndex = 0;
                data->currentAlbum->count = atoi(atts[index]);
                if (data->currentAlbum->count > 0) {
                    data->currentAlbum->songs = (char**) malloc(
                        data->currentAlbum->count * sizeof(char*));
                }
            } else if (strcmp(atts[index], "albumArt") == 0) {
                ++index;
                copyString(&data->currentAlbum->albumArt, atts[index]);
            }
            ++index;
        }
    }

    data->valueBuffer[0] = '\0';
}

void
endXml(void* userData, const char* name)
{
    ExpatUserData *data = (ExpatUserData*) userData;
    FileInfo *file = 0;

    if (strcmp(name, "FileList") == 0) {
        data->currentFile = 0;
    } else if (strcmp(name, "AlbumList") == 0) {
        data->currentAlbum = 0;
    } else if (data->currentFile != 0) {
        if (strcmp(name, "filePath") == 0) {
            copyString(&data->currentFile->filePath, data->valueBuffer);
        } else if (strcmp(name, "title") == 0) {
            copyString(&data->currentFile->title, data->valueBuffer);
        } else if (strcmp(name, "album") == 0) {
            copyString(&data->currentFile->album, data->valueBuffer);
        } else if (strcmp(name, "artist") == 0) {
            copyString(&data->currentFile->artist, data->valueBuffer);
        } else if (strcmp(name, "genre") == 0) {
            copyString(&data->currentFile->genre, data->valueBuffer);
        } else if (strcmp(name, "year") == 0) {
            data->currentFile->year = atoi(data->valueBuffer);
        } else if (strcmp(name, "frameCount") == 0) {
            data->currentFile->frameCount = atoi(data->valueBuffer);
        } else if (strcmp(name, "comment") == 0) {
            copyString(&data->currentFile->comment, data->valueBuffer);
        } else if (strcmp(name, "albumArt") == 0) {
            copyString(&data->currentFile->albumArt, data->valueBuffer);
        } else if (strcmp(name, "file") == 0) {
            ++data->currentFile;
            if (data->currentFile >=
                    data->fileList->list + data->fileList->count) {
                data->currentFile = 0;
            }
        }
    } else if (data->currentAlbum != 0) {
        if (strcmp(name, "algorithm") == 0) {
            copyString(&data->currentAlbum->algorithm, data->valueBuffer);
        } else if (strcmp(name, "song") == 0 &&
                data->currentIndex < data->currentAlbum->count) {
            file = fileById(data->valueBuffer, data->fileList);
            if (file != 0) {
                data->currentAlbum->songs[data->currentIndex] = file->id;
            } else {
                copyString(&data->currentAlbum->songs[data->currentIndex],
                    data->valueBuffer);
            }
            ++data->currentIndex;
        } else if (strcmp(name, "Album") == 0) {
            ++data->currentAlbum;
            if (data->currentAlbum >=
                    data->albumList->list + data->albumList->count) {
                data->currentAlbum = 0;
            }
        }
    }

    data->valueBuffer[0] = '\0';
}

void
valueXml(void* userData, const char* val, int len)
{
    ExpatUserData *data = (ExpatUserData*) userData;
    unsigned int oldSize = strlen(data->valueBuffer);
    unsigned int length = len < BUFFER_SIZE - oldSize ? len :
        BUFFER_SIZE - oldSize;

    strncpy(data->valueBuffer + oldSize, val, length);
    data->valueBuffer[oldSize + length] = '\0';
}

void
copyString(char** destination, const char* source)
{
    *destination = (char*) malloc((strlen(source) + 1) * sizeof(char));
    strcpy(*destination, source);
    (*destination)[strlen(source)] = '\0';
}

FileInfo*
fileById(const char* id, FileList* list)
{
    FileInfo *result = 0;
    FileInfo *curr = 0;
    unsigned int index = 0;
    unsigned int count = 0;

    if (list != 0) {
        curr = list->list;
        count = list->count;
    }

    while (index < count) {
        if (strcmp(curr->id, id) == 0) {
            result = curr;
            break;
        }

        ++curr;
        ++index;
    }

    return result;
}
