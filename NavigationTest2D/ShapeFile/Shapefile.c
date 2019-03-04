/*
The MIT License (MIT)

Copyright (c) 2012 Oneironautics (Mike Smith) https://oneironautics.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Shapefile-internal.h"

/*
int32_t byteswap32(int32_t value)

A cross-platform helper for flipping endianess.

Arguments:
    value: the un-byteswapped value.

Returns:
    int32_t: the byteswapped value.
*/
int32_t byteswap32(int32_t value)
{
#ifdef _WIN32
    value = _byteswap_ulong(value);
#else
    value = __builtin_bswap32(value);
#endif

    return value;
}

/*
void print_msg(const char* format, ...)

A cross-platform helper for sprintf to use sprintf_s on Windows platforms.

Arguments:
    format: the printf-esque format string.
    ...: varargs of the printf formatters.

Returns:
    N/A.
*/
void print_msg(const char* format, ...)
{
    char buf[1024];

    va_list args;
    va_start(args, format);

#ifdef _WIN32
    vsprintf_s(buf, 1024, format, args);
#else
    vsprintf(buf, format, args);
#endif

    printf(buf);
    fflush(stdout);

    va_end(args);
}

/*
const char* shape_type_to_name(const int32_t shape_type)

Converts a numeric shape type to its corresponding string representation.

Arguments:
    const int32_t shape_type: the shape to be named.

Returns:
    A const char* corresponding to the name of the shape type.
*/
const char* shape_type_to_name(const int32_t shape_type)
{
    switch ( shape_type ) {
        case stNull:
            return "Null";
        case stPoint:
            return "Point";
        case stPolyline:
            return "Polyline";
        case stPolygon:
            return "Polygon";
        case stMultiPoint:
            return "MultiPoint";
        case stPointZ:
            return "PointZ";
        case stPolyLineZ:
            return "PolyLineZ";
        case stPolygonZ:
            return "PolygonZ";
        case stMultiPointZ:
            return "MultiPointZ";
        case stPointM:
            return "PointM";
        case stPolyLineM:
            return "PolyLineM";
        case stPolygonM:
            return "PolygonM";
        case stMultiPointM:
            return "MultiPointM";
        case stMultiPatch:
            return "MultiPatch";
        default:
            return "Unknown";
    }
}

/*
FILE* open_shapefile(const char* path)

Opens a shapefile for reading. The caller is responsible for closing the file via
close_shapefile() when it is no longer necessary.

Arguments:
    const char* path: the path to the shapefile to open.

Returns:
    FILE*: a file pointer to the open shapefile.
    NULL: the shapefile could not be opened or was not a shapefile.
*/
FILE* open_shapefile(const char* path)
{
    SFFileHeader header;

    FILE* pShapefile;

#ifdef _WIN32
    fopen_s(&pShapefile, path, "rb");
#else
    pShapefile = fopen(path, "rb");
#endif

    if ( pShapefile == NULL ) {
        print_msg("Could not open shape file <%s>.", path);
        return NULL;
    }

    fread(&header, sizeof(SFFileHeader), 1, pShapefile);

    if ( byteswap32(header.file_code) != SHAPEFILE_FILE_CODE || header.version != SHAPEFILE_VERSION ) {
        print_msg("File <%s> is not a shape file.\n", path);
        return NULL;
    }

    return pShapefile;
}

/*
void close_shapefile(void)

Closes the shapefile.

Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().

Returns:
    N/A.
*/
void close_shapefile(FILE* pShapefile)
{
    if ( pShapefile != NULL ) {
        fclose(pShapefile);
        pShapefile = NULL;
    }
}

/*
void free_shapes(SFShapes* pShapes)

Frees all memory associated with the SFShapes*.

Arguments:
    SFShapes* pShapes: the SFShapes* to free.

Returns:
    N/A.
*/
void free_shapes(SFShapes* pShapes)
{
    uint32_t x = 0;

    if ( pShapes != NULL ) {
        for ( ; x < pShapes->num_records; ++x ) {
            free(pShapes->records[x]);
            pShapes->records[x] = NULL;
        }

        free(pShapes->records);
        free(pShapes);
        pShapes = NULL;
    }
}

/*
const char* get_shapefile_type(void)

Returns the shapefile's shape type as defined in the file header.

Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().

Returns:
    const char*: a string describing the file's shape type.
*/
const char* get_shapefile_type(FILE* pShapefile)
{
    int32_t pos = 0;
    SFFileHeader header;

    pos = ftell(pShapefile);
    fseek(pShapefile, 0, SEEK_SET);
    fread(&header, sizeof(SFFileHeader), 1, pShapefile);
    fseek(pShapefile, pos, SEEK_SET);

    return shape_type_to_name(header.shape_type);
}

/*
SFShapes* allocate_shapes(void)

Allocates memory for shapefile records read from the shapefile.

Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().

Returns:
    SFShapes*: an allocated structure of shape records.
    NULL: an out of memory condition was encountered.
*/
SFShapes* allocate_shapes(FILE* pShapefile)
{
    int32_t x = 0;
    int32_t num_records = 0;
    int32_t content_length = 0;
    SFShapes* pShapes = NULL;

    /*  Read each shape record header and tally up the content length/number of records. */
    while ( !feof(pShapefile) ) {
        SFShapeRecordHeader header;
        int32_t shape_type = 0;

        fread(&header, sizeof(SFShapeRecordHeader), 1, pShapefile);

        if ( feof(pShapefile) ) {
            break;
        }

        header.content_length = byteswap32(header.content_length);
        header.record_number = byteswap32(header.record_number);
        fread(&shape_type, sizeof(int32_t), 1, pShapefile);
        /*  Note: content_length is the number of 16 bit numbers, not a byte count. Multiply by sizeof(int16_t). */
        fseek(pShapefile, header.content_length * sizeof(int16_t)-sizeof(int32_t), SEEK_CUR);
        content_length += header.content_length * sizeof(int16_t) - sizeof(int32_t);
        num_records++;
    }

    /*  Seek back to the end of the main file header. */
    fseek(pShapefile, sizeof(SFFileHeader), SEEK_SET);

    /*  Allocate enough memory for the record index. */
    pShapes = (SFShapes*)malloc(sizeof(SFShapes));

    if ( pShapes == NULL ) {
        print_msg("Could not allocate memory for shapes!");
        return NULL;
    }

    pShapes->records = (SFShapeRecord**)malloc(sizeof(SFShapeRecord)* (num_records + 1));

    if ( pShapes->records == NULL ) {
        print_msg("Could not allocate memory for g_shapes->records!");
        return NULL;
    }

    for ( x = 0; x < num_records; ++x ) {
        pShapes->records[x] = (SFShapeRecord*)malloc(sizeof(SFShapeRecord));

        if ( pShapes->records[x] == NULL ) {
            print_msg("Could not allocate memory for g_shapes->records[%d]!", x);
            return NULL;
        }
    }

    pShapes->num_records = num_records;

    return pShapes;
}

/*
SFShapes* read_shapes(FILE* pShapefile)

Reads shapes from a shapefile.

Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().

Returns:
    SFShapes*: an allocated structure of shape records.
    NULL: an out of memory condition was encountered.
*/
SFShapes* read_shapes(FILE* pShapefile)
{
    int32_t index = 0;

    SFShapes* pShapes = allocate_shapes(pShapefile);

    if ( pShapes == NULL ) {
        return NULL;
    }

    while ( !feof(pShapefile) ) {
        /*  Read a file record header. */
        SFShapeRecordHeader header;
        int32_t shape_type = 0;

        fread(&header, sizeof(SFShapeRecordHeader), 1, pShapefile);

        if ( feof(pShapefile) ) {
            break;
        }

        header.content_length = byteswap32(header.content_length);
        header.record_number = byteswap32(header.record_number);
        fread(&shape_type, sizeof(int32_t), 1, pShapefile);

#ifdef DEBUG
        print_msg("Record %d, length %d (%d bytes), %s.\n", header.record_number, header.content_length, header.content_length * sizeof(int16_t), shape_type_to_name(shape_type));
#endif
        /*  Note: content_length is the number of 16 bit numbers, not a byte count. Multiply by sizeof(int16_t). */
        pShapes->records[index]->record_size = header.content_length * sizeof(int16_t)-sizeof(int32_t);
        pShapes->records[index]->record_type = shape_type;
        pShapes->records[index]->record_offset = ftell(pShapefile);
        fseek(pShapefile, header.content_length * sizeof(int16_t)-sizeof(int32_t), SEEK_CUR);
        index++;
    }

    return pShapes;
}

/*
const SFShapeRecord* get_shape_record(const uint32_t index)

Retrieves an SFShapeRecord* at the specified index.

Arguments:
    const uint32_t index: the index of the shape record to retrieve.

Returns:
    const SFShapeRecord*: the requested shape record.
    NULL: the index was invalid.
*/
const SFShapeRecord* get_shape_record(const SFShapes* pShapes, const uint32_t index)
{
    if ( index >= pShapes->num_records ) {
        return NULL;
    }

    return pShapes->records[index];
}

/*
SFNull* get_null_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a Null shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_null_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFNull* data to return.

Returns:
    SFNull*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFNull* get_null_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    SFNull* null = NULL;

    if ( pRecord->record_type != stNull ) {
        return NULL;
    }

    null = (SFNull*)malloc(sizeof(SFNull));

    if ( null == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(&null->shape_type, sizeof(SFNull), 1, pShapefile);

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
#endif

    return null;
}

/*
SFPoint* get_point_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a Point shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_point_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPoint* data to return.

Returns:
    SFPoint*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPoint* get_point_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    SFPoint* point = NULL;

    if ( pRecord->record_type != stPoint ) {
        return NULL;
    }

    point = (SFPoint*)malloc(sizeof(SFPoint));

    if ( point == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(&point, sizeof(SFPoint), 1, pShapefile);

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tPoint values:\n");
    print_msg("\t\tx => %lf\n", point->x);
    print_msg("\t\ty => %lf\n", point->y);
#endif

    return point;
}

/*
const SFMultiPoint* get_multipoint_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a MultiPoint shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_multipoint_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFMultiPoint* data to return.

Returns:
    SFMultiPoint*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFMultiPoint* get_multipoint_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    SFMultiPoint* multipoint = NULL;

    if ( pRecord->record_type != stMultiPoint ) {
        return NULL;
    }

    multipoint = (SFMultiPoint*)malloc(sizeof(SFMultiPoint));

    if ( multipoint == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(multipoint->box, sizeof(multipoint->box), 1, pShapefile);
    fread(&multipoint->num_points, sizeof(int32_t), 1, pShapefile);

    multipoint->points = (SFPoint*)malloc(sizeof(SFPoint) * multipoint->num_points);
    fread(multipoint->points, sizeof(SFPoint) * multipoint->num_points, 1, pShapefile);

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for (int32_t x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, multipoint->box[x]);
    }

    print_msg("\tPoints: %d\n", multipoint->num_points);

    for (int32_t x = 0; x < multipoint->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, multipoint->points[x].x, multipoint->points[x].y);
    }
#endif

    return multipoint;
}

/*
SFPolyLine* get_polyline_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PolyLine shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_polyline_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPolyLine* data to return.

Returns:
    SFPolyLine*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPolyLine* get_polyline_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFPolyLine* polyline = NULL;

    if ( pRecord->record_type != stPolyline ) {
        return NULL;
    }

    polyline = (SFPolyLine*)malloc(sizeof(SFPolyLine));

    if ( polyline == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(polyline->box, sizeof(polyline->box), 1, pShapefile);
    fread(&polyline->num_parts, sizeof(int32_t), 1, pShapefile);
    fread(&polyline->num_points, sizeof(int32_t), 1, pShapefile);

    polyline->parts = (int32_t*)malloc(sizeof(int32_t) * polyline->num_parts);
    polyline->points = (SFPoint*)malloc(sizeof(SFPoint) * polyline->num_points);

    for ( x = 0; x < polyline->num_parts; ++x ) {
        fread(&polyline->parts[x], sizeof(int32_t), 1, pShapefile);
    }

    for ( x = 0; x < polyline->num_points; ++x ) {
        fread(&polyline->points[x], sizeof(SFPoint), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, polyline->box[x]);
    }

    print_msg("\tParts: %d\n", polyline->num_parts);

    for ( x = 0; x < polyline->num_parts; ++x ) {
        print_msg("\t\tPart [%d] => %d\n", x, polyline->parts[x]);
    }

    print_msg("\tPoints: %d\n", polyline->num_points);

    for ( x = 0; x < polyline->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, polyline->points[x].x, polyline->points[x].y);
    }
#endif

    return polyline;
}

/*
SFPolygon* get_polygon_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a Polygon shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_polygon_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPolygon* data to return.

Returns:
    SFPolygon*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPolygon* get_polygon_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFPolygon* polygon = NULL;

    if ( pRecord->record_type != stPolygon ) {
        return NULL;
    }

    polygon = (SFPolygon*)malloc(sizeof(SFPolygon));

    if ( polygon == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(polygon->box, sizeof(polygon->box), 1, pShapefile);
    fread(&polygon->num_parts, sizeof(int32_t), 1, pShapefile);
    fread(&polygon->num_points, sizeof(int32_t), 1, pShapefile);

    polygon->parts = (int32_t*)malloc(sizeof(int32_t) * polygon->num_parts);
    polygon->points = (SFPoint*)malloc(sizeof(SFPoint) * polygon->num_points);

    for ( x = 0; x < polygon->num_parts; ++x ) {
        fread(&polygon->parts[x], sizeof(int32_t), 1, pShapefile);
    }

    for ( x = 0; x < polygon->num_points; ++x ) {
        fread(&polygon->points[x], sizeof(SFPoint), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, polygon->box[x]);
    }

    print_msg("\tParts: %d\n", polygon->num_parts);

    for ( x = 0; x < polygon->num_parts; ++x ) {
        print_msg("\t\tPart [%d] => %d\n", x, polygon->parts[x]);
    }

    print_msg("\tPoints: %d\n", polygon->num_points);

    for ( x = 0; x < polygon->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, polygon->points[x].x, polygon->points[x].y);
    }
#endif

    return polygon;
}

/*
SFPointM* get_pointm_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PointM shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_pointm_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPointM* data to return.

Returns:
    SFPointM*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPointM* get_pointm_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    SFPointM* pointm = NULL;

    if ( pRecord->record_type != stPointM ) {
        return NULL;
    }

    pointm = (SFPointM*)malloc(sizeof(SFPointM));

    if ( pointm == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(&pointm, sizeof(SFPointM), 1, pShapefile);

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tPointM values:\n");
    print_msg("\t\tx => %lf\n", pointm->x);
    print_msg("\t\ty => %lf\n", pointm->y);
#endif

    return pointm;
}

/*
SFMultiPointM* get_multipointm_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a MultiPointM shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_multipointm_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFMultiPointM* data to return.

Returns:
    SFMultiPointM*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFMultiPointM* get_multipointm_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFMultiPointM* multipointm = NULL;

    if ( pRecord->record_type != stMultiPointM ) {
        return NULL;
    }

    multipointm = (SFMultiPointM*)malloc(sizeof(SFMultiPointM));

    if ( multipointm == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(multipointm->box, sizeof(multipointm->box), 1, pShapefile);
    fread(&multipointm->num_points, sizeof(int32_t), 1, pShapefile);

    multipointm->points = (SFPoint*)malloc(sizeof(SFPoint) * multipointm->num_points);
    fread(multipointm->points, sizeof(SFPoint) * multipointm->num_points, 1, pShapefile);

    fread(multipointm->m_range, sizeof(multipointm->m_range), 1, pShapefile);

    for ( x = 0; x < multipointm->num_points; ++x ) {
        fread(&multipointm->m_array[x], sizeof(double), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, multipointm->box[x]);
    }

    print_msg("\tPoints: %d\n", multipointm->num_points);

    for ( x = 0; x < multipointm->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, multipointm->points[x].x, multipointm->points[x].y);
    }

    print_msg("\tM range: %lf - %lf\n", multipointm->m_range[0], multipointm->m_range[1]);

    for ( x = 0; x < multipointm->num_points; ++x ) {
        print_msg("\t\tM value for point [%d] => %lf\n", x, multipointm->m_array[x]);
    }
#endif

    return multipointm;
}

/*
SFPolyLineM* get_polylinem_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PolyLineM shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_polylinem_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPolyLineM* data to return.

Returns:
    SFPolyLineM*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPolyLineM* get_polylinem_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFPolyLineM* polylinem = NULL;

    if ( pRecord->record_type != stPolyLineM ) {
        return NULL;
    }

    polylinem = (SFPolyLineM*)malloc(sizeof(SFPolyLineM));

    if ( polylinem == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(polylinem->box, sizeof(polylinem->box), 1, pShapefile);
    fread(&polylinem->num_parts, sizeof(int32_t), 1, pShapefile);
    fread(&polylinem->num_points, sizeof(int32_t), 1, pShapefile);

    polylinem->parts = (int32_t*)malloc(sizeof(int32_t) * polylinem->num_parts);
    polylinem->points = (SFPoint*)malloc(sizeof(SFPoint) * polylinem->num_points);

    for ( x = 0; x < polylinem->num_parts; ++x ) {
        fread(&polylinem->parts[x], sizeof(int32_t), 1, pShapefile);
    }

    for ( x = 0; x < polylinem->num_points; ++x ) {
        fread(&polylinem->points[x], sizeof(SFPoint), 1, pShapefile);
    }

    fread(polylinem->m_range, sizeof(polylinem->m_range), 1, pShapefile);

    for ( x = 0; x < polylinem->num_points; ++x ) {
        fread(&polylinem->m_array[x], sizeof(double), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, polylinem->box[x]);
    }

    print_msg("\tParts: %d\n", polylinem->num_parts);

    for ( x = 0; x < polylinem->num_parts; ++x ) {
        print_msg("\t\tPart [%d] => %d\n", x, polylinem->parts[x]);
    }

    print_msg("\tPoints: %d\n", polylinem->num_points);

    for ( x = 0; x < polylinem->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, polylinem->points[x].x, polylinem->points[x].y);
    }

    print_msg("\tM range: %lf - %lf\n", polylinem->m_range[0], polylinem->m_range[1]);

    for ( x = 0; x < polylinem->num_points; ++x ) {
        print_msg("\t\tM value for point [%d] => %lf\n", x, polylinem->m_array[x]);
    }
#endif

    return polylinem;
}

/*
SFPolygonM* get_polygonm_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PolygonM shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_polygonm_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPolygonM* data to return.

Returns:
    SFPolygonM*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPolygonM* get_polygonm_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFPolygonM* polygonm = NULL;

    if ( pRecord->record_type != stPolygonM ) {
        return NULL;
    }

    polygonm = (SFPolygonM*)malloc(sizeof(SFPolygonM));

    if ( polygonm == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(polygonm->box, sizeof(polygonm->box), 1, pShapefile);
    fread(&polygonm->num_parts, sizeof(int32_t), 1, pShapefile);
    fread(&polygonm->num_points, sizeof(int32_t), 1, pShapefile);

    polygonm->parts = (int32_t*)malloc(sizeof(int32_t) * polygonm->num_parts);
    polygonm->points = (SFPoint*)malloc(sizeof(SFPoint) * polygonm->num_points);

    for ( x = 0; x < polygonm->num_parts; ++x ) {
        fread(&polygonm->parts[x], sizeof(int32_t), 1, pShapefile);
    }

    for ( x = 0; x < polygonm->num_points; ++x ) {
        fread(&polygonm->points[x], sizeof(SFPoint), 1, pShapefile);
    }

    fread(polygonm->m_range, sizeof(polygonm->m_range), 1, pShapefile);

    for ( x = 0; x < polygonm->num_points; ++x ) {
        fread(&polygonm->m_array[x], sizeof(double), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, polygonm->box[x]);
    }

    print_msg("\tParts: %d\n", polygonm->num_parts);

    for ( x = 0; x < polygonm->num_parts; ++x ) {
        print_msg("\t\tPart [%d] => %d\n", x, polygonm->parts[x]);
    }

    print_msg("\tPoints: %d\n", polygonm->num_points);

    for ( x = 0; x < polygonm->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, polygonm->points[x].x, polygonm->points[x].y);
    }

    print_msg("\tM range: %lf - %lf\n", polygonm->m_range[0], polygonm->m_range[1]);

    for ( x = 0; x < polygonm->num_points; ++x ) {
        print_msg("\t\tM value for point [%d] => %lf\n", x, polygonm->m_array[x]);
    }
#endif

    return polygonm;
}

/*
SFPointZ* get_pointz_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PointZ shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_pointz_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPointZ* data to return.

Returns:
    SFPointZ*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPointZ* get_pointz_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    SFPointZ* pointz = NULL;

    if ( pRecord->record_type != stPointZ ) {
        return NULL;
    }

    pointz = (SFPointZ*)malloc(sizeof(SFPointZ));

    if ( pointz == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(&pointz, sizeof(SFPointZ), 1, pShapefile);

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\t\tx => %lf\n", pointz->x);
    print_msg("\t\ty => %lf\n", pointz->y);
#endif

    return pointz;
}

/*
SFMultiPointZ* get_multipointz_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a MultiPointZ shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_multipointz_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFMultiPointZ* data to return.

Returns:
    SFMultiPointZ*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFMultiPointZ* get_multipointz_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFMultiPointZ* multipointz = NULL;

    if ( pRecord->record_type != stMultiPointZ ) {
        return NULL;
    }

    multipointz = (SFMultiPointZ*)malloc(sizeof(SFMultiPointZ));

    if ( multipointz == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(multipointz->box, sizeof(multipointz->box), 1, pShapefile);
    fread(&multipointz->num_points, sizeof(int32_t), 1, pShapefile);

    multipointz->points = (SFPoint*)malloc(sizeof(SFPoint) * multipointz->num_points);
    fread(multipointz->points, sizeof(SFPoint) * multipointz->num_points, 1, pShapefile);

    fread(multipointz->m_range, sizeof(multipointz->m_range), 1, pShapefile);

    for ( x = 0; x < multipointz->num_points; ++x ) {
        fread(&multipointz->m_array[x], sizeof(double), 1, pShapefile);
    }

    fread(multipointz->m_range, sizeof(multipointz->m_range), 1, pShapefile);

    for ( x = 0; x < multipointz->num_points; ++x ) {
        fread(&multipointz->m_array[x], sizeof(double), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, multipointz->box[x]);
    }

    print_msg("\tPoints: %d\n", multipointz->num_points);

    for ( x = 0; x < multipointz->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, multipointz->points[x].x, multipointz->points[x].y);
    }

    print_msg("\tZ range: %lf - %lf\n", multipointz->z_range[0], multipointz->z_range[1]);

    for ( x = 0; x < multipointz->num_points; ++x ) {
        print_msg("\t\tZ value for point [%d] => %lf\n", x, multipointz->z_array[x]);
    }

    print_msg("\tM range: %lf - %lf\n", multipointz->m_range[0], multipointz->m_range[1]);

    for ( x = 0; x < multipointz->num_points; ++x ) {
        print_msg("\t\tM value for point [%d] => %lf\n", x, multipointz->m_array[x]);
    }
#endif

    return multipointz;
}

/*
const SFPolyLineZ* get_polylinez_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PolyLineZ shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_polylinez_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPolyLineZ* data to return.

Returns:
    SFPolyLineZ*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPolyLineZ* get_polylinez_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFPolyLineZ* polylinez = NULL;

    if ( pRecord->record_type != stPolygonZ ) {
        return NULL;
    }

    polylinez = (SFPolyLineZ*)malloc(sizeof(SFPolyLineZ));

    if ( polylinez == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(polylinez->box, sizeof(polylinez->box), 1, pShapefile);
    fread(&polylinez->num_parts, sizeof(int32_t), 1, pShapefile);
    fread(&polylinez->num_points, sizeof(int32_t), 1, pShapefile);

    polylinez->parts = (int32_t*)malloc(sizeof(int32_t) * polylinez->num_parts);
    polylinez->points = (SFPoint*)malloc(sizeof(SFPoint) * polylinez->num_points);
    polylinez->z_array = (double*)malloc(sizeof(double) * polylinez->num_points);
    polylinez->m_array = (double*)malloc(sizeof(double) * polylinez->num_points);

    for ( x = 0; x < polylinez->num_parts; ++x ) {
        fread(&polylinez->parts[x], sizeof(int32_t), 1, pShapefile);
    }

    for ( x = 0; x < polylinez->num_points; ++x ) {
        fread(&polylinez->points[x], sizeof(SFPoint), 1, pShapefile);
    }

    fread(polylinez->z_range, sizeof(polylinez->z_range), 1, pShapefile);

    for ( x = 0; x < polylinez->num_points; ++x ) {
        fread(&polylinez->z_array[x], sizeof(double), 1, pShapefile);
    }

    fread(polylinez->m_range, sizeof(polylinez->m_range), 1, pShapefile);

    for ( x = 0; x < polylinez->num_points; ++x ) {
        fread(&polylinez->m_array[x], sizeof(double), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, polylinez->box[x]);
    }

    print_msg("\tParts: %d\n", polylinez->num_parts);

    for ( x = 0; x < polylinez->num_parts; ++x ) {
        print_msg("\t\tPart [%d] => %d\n", x, polylinez->parts[x]);
    }

    print_msg("\tPoints: %d\n", polylinez->num_points);

    for ( x = 0; x < polylinez->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, polylinez->points[x].x, polylinez->points[x].y);
    }

    print_msg("\tZ range: %lf - %lf\n", polylinez->z_range[0], polylinez->z_range[1]);

    for ( x = 0; x < polylinez->num_points; ++x ) {
        print_msg("\t\tZ value for point [%d] => %lf\n", x, polylinez->z_array[x]);
    }

    print_msg("\tM range: %lf - %lf\n", polylinez->m_range[0], polylinez->m_range[1]);

    for ( x = 0; x < polylinez->num_points; ++x ) {
        print_msg("\t\tM value for point [%d] => %lf\n", x, polylinez->m_array[x]);
    }
#endif

    return polylinez;
}

/*
SFPolygonZ* get_polygonz_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a PolygonZ shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_polygonz_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFPolygonZ* data to return.

Returns:
    SFPolygonZ*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFPolygonZ* get_polygonz_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    int32_t x = 0;
    SFPolygonZ* polygonz = NULL;

    if ( pRecord->record_type != stPolygonZ ) {
        return NULL;
    }

    polygonz = (SFPolygonZ*)malloc(sizeof(SFPolygonZ));

    if ( polygonz == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);
    fread(polygonz->box, sizeof(polygonz->box), 1, pShapefile);
    fread(&polygonz->num_parts, sizeof(int32_t), 1, pShapefile);
    fread(&polygonz->num_points, sizeof(int32_t), 1, pShapefile);

    polygonz->parts = (int32_t*)malloc(sizeof(int32_t) * polygonz->num_parts);
    polygonz->points = (SFPoint*)malloc(sizeof(SFPoint) * polygonz->num_points);
    polygonz->z_array = (double*)malloc(sizeof(double) * polygonz->num_points);
    polygonz->m_array = (double*)malloc(sizeof(double) * polygonz->num_points);

    for ( x = 0; x < polygonz->num_parts; ++x ) {
        fread(&polygonz->parts[x], sizeof(int32_t), 1, pShapefile);
    }

    for ( x = 0; x < polygonz->num_points; ++x ) {
        fread(&polygonz->points[x], sizeof(SFPoint), 1, pShapefile);
    }

    fread(polygonz->z_range, sizeof(polygonz->z_range), 1, pShapefile);

    for ( x = 0; x < polygonz->num_points; ++x ) {
        fread(&polygonz->z_array[x], sizeof(double), 1, pShapefile);
    }

    fread(polygonz->m_range, sizeof(polygonz->m_range), 1, pShapefile);

    for ( x = 0; x < polygonz->num_points; ++x ) {
        fread(&polygonz->m_array[x], sizeof(double), 1, pShapefile);
    }

#ifdef DEBUG
    print_msg("Data length: %d, shape type: %s\n", pRecord->record_size, shape_type_to_name(pRecord->record_type));
    print_msg("\tBox values:\n");

    for ( x = 0; x < 4; ++x ) {
        print_msg("\t\tBox [%d] => %lf\n", x, polygonz->box[x]);
    }

    print_msg("\tParts: %d\n", polygonz->num_parts);

    for ( x = 0; x < polygonz->num_parts; ++x ) {
        print_msg("\t\tPart [%d] => %d\n", x, polygonz->parts[x]);
    }

    print_msg("\tPoints: %d\n", polygonz->num_points);

    for ( x = 0; x < polygonz->num_points; ++x ) {
        print_msg("\t\tPoint [%d] => x: %lf, y: %lf\n", x, polygonz->points[x].x, polygonz->points[x].y);
    }

    print_msg("\tZ range: %lf - %lf\n", polygonz->z_range[0], polygonz->z_range[1]);

    for ( x = 0; x < polygonz->num_points; ++x ) {
        print_msg("\t\tZ value for point [%d] => %lf\n", x, polygonz->z_array[x]);
    }

    print_msg("\tM range: %lf - %lf\n", polygonz->m_range[0], polygonz->m_range[1]);

    for ( x = 0; x < polygonz->num_points; ++x ) {
        print_msg("\t\tM value for point [%d] => %lf\n", x, polygonz->m_array[x]);
    }
#endif

    return polygonz;
}

/*
SFMultiPatch* get_multipatch_shape(FILE* pShapefile, const SFShapeRecord* pRecord)

Retrieves a MultiPatch shape from the specified record. The caller is responsible for freeing the returned pointer
with a call to free_multipatch_shape().


Arguments:
    FILE* pShapefile: a file pointer to a file opened by open_shapefile().
    const SFShapeRecord* pRecord: the record that contains SFMultiPatch* data to return.

Returns:
    SFMultiPatch*: the shape.
    NULL: the record type did not match, or an out of memory condition was encountered.
*/
SFMultiPatch* get_multipatch_shape(FILE* pShapefile, const SFShapeRecord* pRecord)
{
    SFMultiPatch* multipatch = NULL;

    if ( pRecord->record_type != stMultiPatch ) {
        return NULL;
    }

    multipatch = (SFMultiPatch*)malloc(sizeof(SFMultiPatch));

    if ( multipatch == NULL ) {
        return NULL;
    }

    fseek(pShapefile, pRecord->record_offset, SEEK_SET);

    return multipatch;
}

/*
void free_null_shape(SFNull* null)

Frees a SFNull* returned by get_null_shape.

Arguments:
    SFNull* null: a SFNull* returned by get_null_shape.

Returns:
    N/A.
*/
void free_null_shape(SFNull* pNull)
{
    if ( pNull != NULL ) {
        free(pNull);
        pNull = NULL;
    }
}

/*
void free_point_shape(SFPoint* pPoint)

Frees a SFPoint* returned by get_point_shape.

Arguments:
    SFPoint* pPoint: a SFPoint* returned by get_point_shape.

Returns:
    N/A.
*/
void free_point_shape(SFPoint* pPoint)
{
    if ( pPoint != NULL ) {
        free(pPoint);
        pPoint = NULL;
    }
}

/*
void free_polyline_shape(SFPolyLine* pPolyline)

Frees a SFPolyLine* returned by get_polyline_shape.

Arguments:
    SFPolyLine* pPolyline: a SFPolyLine* returned by get_polyline_shape.

Returns:
    N/A.
*/
void free_polyline_shape(SFPolyLine* pPolyline)
{
    if ( pPolyline != NULL ) {
        free(pPolyline->parts);
        pPolyline->parts = NULL;
        free(pPolyline->points);
        pPolyline->points = NULL;
        free(pPolyline);
        pPolyline = NULL;
    }
}

/*
void free_polygon_shape(SFPolygon* polygon)

Frees a SFPolygon* returned by get_polygon_shape.

Arguments:
    SFPolygon* polygon: a SFPolygon* returned by get_polygon_shape.

Returns:
    N/A.
*/
void free_polygon_shape(SFPolygon* pPolygon)
{
    if ( pPolygon != NULL ) {
        free(pPolygon->parts);
        pPolygon->parts = NULL;
        free(pPolygon->points);
        pPolygon->points = NULL;
        free(pPolygon);
        pPolygon = NULL;
    }
}

/*
void free_multipoint_shape(SFMultiPoint* pMultipoint)

Frees a SFMultiPoint* returned by get_multipoint_shape.

Arguments:
    SFMultiPoint* pMultipoint: a SFMultiPoint* returned by get_multipoint_shape.

Returns:
    N/A.
*/
void free_multipoint_shape(SFMultiPoint* pMultipoint)
{
    if ( pMultipoint != NULL ) {
        free(pMultipoint->points);
        pMultipoint->points = NULL;
        free(pMultipoint);
        pMultipoint = NULL;
    }
}

/*
void free_pointz_shape(SFPointZ* pPointz)

Frees a SFPointZ* returned by get_pointz_shape.

Arguments:
    SFPointZ* pPointz: a SFPointZ* returned by get_pointz_shape.

Returns:
    N/A.
*/
void free_pointz_shape(SFPointZ* pPointz)
{
    if ( pPointz != NULL ) {
        free(pPointz);
        pPointz = NULL;
    }
}

/*
void free_polylinez_shape(SFPolyLineZ* pPolylinez)

Frees a SFPolyLineZ* returned by get_polylinez_shape.

Arguments:
    SFPolyLineZ* pPolylinez: a SFPolyLineZ* returned by get_polylinez_shape.

Returns:
    N/A.
*/
void free_polylinez_shape(SFPolyLineZ* pPolylinez)
{
    if ( pPolylinez != NULL ) {
        free(pPolylinez);
        pPolylinez = NULL;
    }
}

/*
void free_polygonz_shape(SFPolygonZ* pPolygonz)

Frees a SFPolygonZ* returned by get_polygonz_shape.

Arguments:
    SFPolygonZ* pPolygonz: a SFPolygonZ* returned by get_polygonz_shape.

Returns:
    N/A.
*/
void free_polygonz_shape(SFPolygonZ* pPolygonz)
{
    if ( pPolygonz != NULL ) {
        free(pPolygonz->parts);
        pPolygonz->parts = NULL;
        free(pPolygonz->points);
        pPolygonz->points = NULL;
        free(pPolygonz->z_array);
        pPolygonz->z_array = NULL;
        free(pPolygonz->m_array);
        pPolygonz->m_array = NULL;
        free(pPolygonz);
        pPolygonz = NULL;
    }
}

/*
void free_multipointz_shape(SFMultiPointZ* pMultipointz)

Frees a SFMultiPointZ* returned by get_multipointz_shape.

Arguments:
    SFMultiPointZ* pMultipointz: a SFMultiPointZ* returned by get_multipointz_shape.

Returns:
    N/A.
*/
void free_multipointz_shape(SFMultiPointZ* pMultipointz)
{
    if ( pMultipointz != NULL ) {
        free(pMultipointz);
        pMultipointz = NULL;
    }
}

/*
void free_pointm_shape(SFPointM* pPointm)

Frees a SFPointM* returned by get_pointm_shape.

Arguments:
    SFPointM* pPointm: a SFPointM* returned by get_pointm_shape.

Returns:
    N/A.
*/
void free_pointm_shape(SFPointM* pPointm)
{
    if ( pPointm != NULL ) {
        free(pPointm);
        pPointm = NULL;
    }
}

/*
void free_polylinem_shape(SFPolyLineM* pPolylinem)

Frees a SFPolyLineM* returned by get_polylinem_shape.

Arguments:
    SFPolyLineM* pPolylinem: a SFPolyLineM* returned by get_polylinem_shape.

Returns:
    N/A.
*/
void free_polylinem_shape(SFPolyLineM* pPolylinem)
{
    if ( pPolylinem != NULL ) {
        free(pPolylinem);
        pPolylinem = NULL;
    }
}

/*
void free_polygonm_shape(SFPolygonM* pPolygonm)

Frees a SFPolygonM* returned by get_polygonm_shape.

Arguments:
    SFPolygonM* pPolygonm: a SFPolygonM* returned by get_polygonm_shape.

Returns:
    N/A.
*/
void free_polygonm_shape(SFPolygonM* pPolygonm)
{
    if ( pPolygonm != NULL ) {
        free(pPolygonm);
        pPolygonm = NULL;
    }
}

/*
void free_multipointm_shape(SFMultiPointM* pMultipointm)

Frees a SFMultiPointM* returned by get_multipointm_shape.

Arguments:
    SFMultiPointM* pMultipointm: a SFMultiPointM* returned by get_multipointm_shape.

Returns:
    N/A.
*/
void free_multipointm_shape(SFMultiPointM* pMultipointm)
{
    if ( pMultipointm != NULL ) {
        free(pMultipointm);
        pMultipointm = NULL;
    }
}

/*
void free_multipatch_shape(SFMultiPatch* pMultipatch)

Frees a SFMultiPatch* returned by get_multipatch_shape.

Arguments:
    SFMultiPatch* pMultipatch: a SFMultiPatch* returned by get_multipatch_shape.

Returns:
    N/A.
*/
void free_multipatch_shape(SFMultiPatch* pMultipatch)
{
    if ( pMultipatch != NULL ) {
        free(pMultipatch);
        pMultipatch = NULL;
    }
}
