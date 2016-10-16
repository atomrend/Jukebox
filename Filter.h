#ifndef FILTER_H
#define FILTER_H

#include "JukeboxPlaylist.h"

/** Filters the file list to produce a playlist that matches the passed filter
 *      string. Valid parameters to match are title, album, artist, genre,
 *      comment, year, and favorite. Quotation is required for multiple word
 *      strings. Valid operators are ==, !=, <, >, <=, >=,
 *      =~ (regular expression, only for strings). Use && (and) and || (or) to
 *      combine multiple conditions. Use parenthese to group conditions.
 *      Example: artist == Metallica && (album == "Master of Puppets" ||
 *      favorite == 1)
 *  @param[out] count - The number of files that match the filter string.
 *  @param[in] filter - The filter string.
 *  @param[in] fileList - The list of files to check against.
 *  @return A list of file IDs separated by the string-terminating-character,
 *      '\0'. This list will need to be freed from the calling function.
 *      Assuming all file IDs are 16 characters in length.
 */
char*
filterPlaylist(
    unsigned int* count,
    const char* const filter,
    const FileList* const fileList);

#endif /* FILTER_H */
