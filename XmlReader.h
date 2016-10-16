#ifndef XML_READER_H
#define XML_READER_H

#include "JukeboxPlaylist.h"

int readPlaylistFile(
    const char* filePath,
    FileList* fileList,
    AlbumList* albumList);

#endif /* XML_READER_H */
