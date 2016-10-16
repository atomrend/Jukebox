#include "Jukebox.h"
#include "Filter.h"
#include "JukeboxPlaylist.h"
#include "mpg123app.h"
#include "XmlReader.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define MESSAGE_SIZE 32
#define RANDOM_NUMBER 2147483647
#define PLAYLIST_PATH "/etc/jukeboxPlaylist.xml"
#define SOCKET_PATH "/var/run/jukebox.socket"
#define PLAY "00"
#define PAUSE "01"
#define NEXT_SONG "02"
#define PREV_SONG "03"
#define SET_ALBUM "04"
#define SET_SONG "05"
#define SET_SHUFFLE "06"
#define SET_REPEAT "07"
#define GET_SEED "08"
#define GET_ALBUM "09"
#define GET_SONG "10"
#define GET_FRAME "11"
#define GET_STATUS "12"

unsigned int randomNumber(unsigned int seed);
FileInfo* findFileById(FileList* fileList, const char* id);
Album* findAlbumById(AlbumList *albumList, const char* id);
int fileAlreadyInPlaylist(FileInfo** playlist, const char* id);
void setupPlaylist(Album* album, FileInfo** playlist, FileList* fileList);

unsigned int
randomNumber(unsigned int seed)
{
    static unsigned int randomNum = 0;

    if (seed == 0) {
        	randomNum = (randomNum * 323727) % 6073;
    } else if (seed == RANDOM_NUMBER) {
    } else {
        randomNum = seed % 6073;
    }

    return randomNum;
}

FileInfo*
findFileById(FileList* fileList, const char* id)
{
    FileInfo *result = 0;
    unsigned int index = 0;
    FileInfo *curr = (FileInfo*) fileList->list;

    while (index < fileList->count) {
        if (strcmp(curr->id, id) == 0) {
            result = curr;
            break;
        }
        ++curr;
        ++index;
    }

    return result;
}

Album*
findAlbumById(AlbumList *albumList, const char* id)
{
    Album *result = 0;
    unsigned int index = 0;
    Album *curr = (Album*) albumList->list;

    while (index < albumList->count) {
        if (strcmp(curr->id, id) == 0) {
            result = curr;
            break;
        }
        ++curr;
        ++index;
    }

    return result;
}

int
fileAlreadyInPlaylist(FileInfo** playlist, const char* id)
{
    int result = -1;
    unsigned int index = 0;

    while (playlist[index] != 0) {
        if (strcmp(playlist[index]->id, id) == 0) {
            result = index;
            break;
        }
        ++index;
    }

    return result;
}

void
setupPlaylist(Album* album, FileInfo** playlist, FileList* fileList)
{
    FileInfo *file = 0;
    char *filtered = 0;
    unsigned int index = 0;
    unsigned int playlistIndex = 0;
    unsigned int count = 0;
    int rand = 0;
    int alreadyPresent = 0;

    if (album != 0) {
        if (album->count == 0 && album->algorithm != 0 &&
                strlen(album->algorithm) > 0 && album->songs == 0) {
            filtered = filterPlaylist(&count, album->algorithm, fileList);
            album->count = count;
            album->songs = (char**) malloc(count * sizeof(char*));

            index = 0;
            while (index < count) {
                album->songs[index] = (char*) calloc(17, sizeof(char));
                strcpy(album->songs[index], filtered + (index * 17));
                ++index;
            }

            free(filtered);
        }

        count = album->count;
        if (album->shuffle == 1) {
            while (count > 0) {
                file = 0;
                while (file == 0) {
                    rand = randomNumber(0);
                    index = count / 6073;
                    while (index > 0) {
                        rand += randomNumber(0);
                        --index;
                    }
                    rand %= count;

                    index = 0;
                    while (rand >= 0) {
                        alreadyPresent = fileAlreadyInPlaylist(playlist,
                            album->songs[index]);
                        if (alreadyPresent == -1) {
                            --rand;
                            if (rand < 0) {
                                break;
                            }
                        }

                        ++index;
                    }

                    if (index < album->count) {
                        file = findFileById(fileList, album->songs[index]);
                    } else {
                        file = 0;
                    }
                }

                if (file != 0) {
                    playlist[playlistIndex] = file;
                    playlist[playlistIndex + 1] = 0;
                    ++playlistIndex;
                    --count;
                }
            }

            playlist[playlistIndex] = 0;
        } else {
            index = 0;
            while (index < count) {
                file = findFileById(fileList, album->songs[index]);
                if (file != 0) {
                    playlist[playlistIndex] = file;
                    ++playlistIndex;
                }

                ++index;
            }

            playlist[playlistIndex] = 0;
        }
    }
}

void
jukebox()
{
    FileList fileList;
    AlbumList albumList;
    char message[MESSAGE_SIZE + 1];
    struct sockaddr_un address;
    FileInfo **playlist = 0;
    Album *album = 0;
    Album *playingAlbum = 0;
    Album *tmpAlbum = 0;
    FileInfo *file = 0;
    socklen_t addressSize = sizeof(struct sockaddr_un);
    unsigned int seed = 0;
    unsigned int frame = 0;
    int index = 0;
    int socketId = 0;
    int connId = 0;
    int receiveSize = 0;
    unsigned char status = 0;
    char proceed = 1;
//FILE *handle = 0; // handle = fopen("/home/web/remote.txt", "a");

    if (readPlaylistFile(PLAYLIST_PATH, &fileList, &albumList) != 0) {
        proceed = 0;
    }

    if (proceed == 1) {
        socketId = socket(PF_UNIX, SOCK_STREAM, 0);
        if (socketId < 0) {
            proceed = 0;
        }
    }

    if (proceed == 1) {
        memset(&address, 0, sizeof(address));
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, SOCKET_PATH);
        unlink(SOCKET_PATH);
        index = bind(socketId, (const struct sockaddr*) &address,
            sizeof(struct sockaddr_un));
        if (index < 0) {
            proceed = 0;
        }
    }

    if (proceed == 1) {
        index = fcntl(socketId, F_GETFL, 0);
        if (index < 0) {
            proceed = 0;
        } else {
            index |= O_NONBLOCK;
            index = fcntl(socketId, F_SETFL, index);
            if (index < 0) {
                proceed = 0;
            }
        }
    }

    if (proceed == 1) {
        index = listen(socketId, 5);
        if (index != 0) {
            proceed = 0;
        }
    }

    if (proceed == 1) {
        playlist = (FileInfo**) calloc(fileList.count + 1, sizeof(FileInfo*));

        seed = time(0) % 6073;
        while (seed == 0) {
            seed = time(0) % 6073;
        }
        randomNumber(seed);

        chmod(SOCKET_PATH, 0777);
    }

    while (proceed == 1) {
        status = 0;
        file = 0;
        frame = 0;

        connId = accept(socketId, (struct sockaddr*) &address, &addressSize);
        if (connId >= 0) {
            receiveSize = read(connId, message, MESSAGE_SIZE);
            message[receiveSize] = '\0';
            if (receiveSize > 0) {
                if (strncmp(message, PLAY, 2) == 0) {
                    status = 1;
                } else if (strncmp(message, SET_ALBUM, 2) == 0) {
                    album = findAlbumById(&albumList, message + 3);
                    if (album != 0) {
                        randomNumber(seed);
                        setupPlaylist(album, playlist, &fileList);
                        index = 0;
                    }
                } else if (strncmp(message, SET_SONG, 2) == 0) {
                    index = fileAlreadyInPlaylist(playlist, message + 3);
                } else if (strncmp(message, SET_SHUFFLE, 2) == 0) {
                    album = findAlbumById(&albumList, message + 6);
                    if (album != 0) {
                        album->shuffle = 0;
                        if (message[3] == '1') {
                            album->shuffle = 1;
                        }
                    }
                    album = 0;
                } else if (strncmp(message, SET_REPEAT, 2) == 0) {
                    album = findAlbumById(&albumList, message + 6);
                    if (album != 0) {
                        album->repeat = 0;
                        if (message[3] == '1') {
                            album->repeat = 1;
                        }
                    }
                    album = 0;
                } else if (strncmp(message, GET_SEED, 2) == 0) {
                    snprintf(message, MESSAGE_SIZE, "seed=%u", seed);
                    receiveSize = write(connId, message, strlen(message) + 1);
                } else if (strncmp(message, GET_ALBUM, 2) == 0) {
                    snprintf(message, MESSAGE_SIZE, "album=0");
                    receiveSize = write(connId, message, strlen(message) + 1);
                } else if (strncmp(message, GET_SONG, 2) == 0) {
                    snprintf(message, MESSAGE_SIZE, "song=0");
                    receiveSize = write(connId, message, strlen(message) + 1);
                } else if (strncmp(message, GET_FRAME, 2) == 0) {
                    snprintf(message, MESSAGE_SIZE, "frame=0");
                    receiveSize = write(connId, message, strlen(message) + 1);
                } else if (strncmp(message, GET_STATUS, 2) == 0) {
                    snprintf(message, MESSAGE_SIZE, "status=0");
                    receiveSize = write(connId, message, strlen(message) + 1);
                }
            } else {
                usleep(100000);
            }
            close(connId);
        } else {
            usleep(100000);
        }

        while (status == 1 && album != 0 && playlist[index] != 0) {
            playingAlbum = album;
            file = playlist[index];
            if (open_track(file->filePath) == 1) {
                status = 1;
                frame = 0;

                while (proceed == 1) {
                    if (status == 1) {
                        if (play_frame() == 0) {
                            break;
                        } // if (play_frame() == 0)
                        ++frame;
                    } // if (status == 1)

                    connId = accept(socketId, (struct sockaddr*) &address,
                        &addressSize);
                    if (connId >= 0) {
                        receiveSize = read(connId, message, MESSAGE_SIZE);
                        message[receiveSize] = '\0';

                        if (receiveSize > 0) {
                            if (strncmp(message, PLAY, 2) == 0) {
                                status = 1;
                                if (album != playingAlbum) {
                                    break;
                                } // if (album != playingAlbum)
                            } else if (strncmp(message, PAUSE, 2) == 0) {
                                status = 2;
                            } else if (strncmp(message, NEXT_SONG, 2) == 0) {
                                break;
                            } else if (strncmp(message, PREV_SONG, 2) == 0) {
                                if (frame > 192 || index == 0) { // 5 seconds.
                                    --index;
                                } else { // if (frame > 192 || index == 0)
                                    index -= 2;
                                } // if (frame > 192 || index == 0)
                                break;
                            } else if (strncmp(message, SET_ALBUM, 2) == 0) {
                                tmpAlbum = findAlbumById(&albumList,
                                    message + 3);
                                if (tmpAlbum != 0) {
                                    album = tmpAlbum;
                                    randomNumber(seed);
                                    setupPlaylist(album, playlist, &fileList);
                                    index = -1;
                                } // if (tmpAlbum != 0)
                            } else if (strncmp(message, SET_SONG, 2) == 0) {
                                receiveSize = fileAlreadyInPlaylist(playlist,
                                    message + 3);
                                if (receiveSize >= 0) {
                                    index = receiveSize - 1;
                                    break;
                                } // if (receiveSize >= 0)
                            } else if (strncmp(message, SET_SHUFFLE, 2) == 0) {
                                tmpAlbum = findAlbumById(&albumList,
                                    message + 6);
                                if (tmpAlbum != 0) {
                                    tmpAlbum->shuffle = 0;
                                    if (message[3] == '1') {
                                        tmpAlbum->shuffle = 1;
                                    }
                                }
                            } else if (strncmp(message, SET_REPEAT, 2) == 0) {
                                tmpAlbum = findAlbumById(&albumList,
                                    message + 6);
                                if (tmpAlbum != 0) {
                                    tmpAlbum->repeat = 0;
                                    if (message[3] == '1') {
                                        tmpAlbum->repeat = 1;
                                    }
                                }
                            } else if (strncmp(message, GET_SEED, 2) == 0) {
                                snprintf(message, MESSAGE_SIZE, "seed=%u",
                                    seed);
                                receiveSize = write(connId, message,
                                    strlen(message) + 1);
                            } else if (strncmp(message, GET_ALBUM, 2) == 0) {
                                snprintf(message, MESSAGE_SIZE, "album=%s",
                                    album->id);
                                receiveSize = write(connId, message,
                                    strlen(message) + 1);
                            } else if (strncmp(message, GET_SONG, 2) == 0) {
                                snprintf(message, MESSAGE_SIZE, "song=%s",
                                    file->id);
                                receiveSize = write(connId, message,
                                    strlen(message) + 1);
                            } else if (strncmp(message, GET_FRAME, 2) == 0) {
                                snprintf(message, MESSAGE_SIZE, "frame=%u",
                                    frame);
                                receiveSize = write(connId, message,
                                    strlen(message) + 1);
                            } else if (strncmp(message, GET_STATUS, 2) == 0) {
                                snprintf(message, MESSAGE_SIZE, "status=%u",
                                    status);
                                receiveSize = write(connId, message,
                                    strlen(message) + 1);
                            } // message conditional
                        } // if (receiveSize > 0)

                        close(connId);
                    } // if (connId >= 0)

                    if (status == 2) {
                        usleep(100000);
                    }
                } // while (proceed == 1)

                close_track();
            } // if (open_track(file->filePath) == 1)

            ++index;

            if (playlist[index] == 0) {
                seed = time(0) % 6073;
                while (seed == 0) {
                    seed = time(0) % 6073;
                }

                if (album->repeat == 1) {
                    if (album->shuffle == 1) {
                        randomNumber(seed);
                        setupPlaylist(album, playlist, &fileList);
                    } // if (album->shuffle == 1)

                    index = 0;
                } // if (album->repeat == 1)
            } // if (playlist[index] == 0)

            if (playlist[index] == 0) {
                album = 0;
            }

            playingAlbum = 0;
        } // while (status == 1 && album != 0 && playlist[index] != 0)
    } // while (proceed == 1)
}
