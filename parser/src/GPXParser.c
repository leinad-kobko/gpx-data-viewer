#include "GPXParser.h"
#include "LinkedListAPI.h"
#include "GPXHelper.h"

GPXdoc* createGPXdoc(char* fileName){

    if(fileName == NULL){
        return NULL;
    }

    /* initialize the GPXdoc*/
    GPXdoc *ptr = malloc(sizeof(GPXdoc));

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     * 
     * Based off the libXmlExample.c file that was provided
     */
    LIBXML_TEST_VERSION
    

    /* Read the GPX file and get the root element to be the head*/
    xmlDoc *doc = xmlReadFile(fileName, NULL, 0);

    /* check if file parsing was successful */
    if(doc == NULL){
        free(ptr);
        return NULL;
    }

    xmlNode *root_element = xmlDocGetRootElement(doc);

    if(root_element == NULL){
        free(ptr);
        return NULL;
    }

    /* Parse for the header information */
    char *namespace = (char*)root_element->ns->href;
    char *version_string = (char*)xmlGetProp(root_element, (const xmlChar *)"version"); 
    double version = atof(version_string);
    free(version_string);
    char *creator = (char*)xmlGetProp(root_element, (const xmlChar *)"creator");
    
    /* Copy the parsed information into the GPXdoc struct */
    strcpy(ptr->namespace, namespace); 
    ptr->version = version;
    ptr->creator = malloc(strlen(creator)+1);
    strcpy(ptr->creator, creator);
    free(creator);
    
    /* Initialize the lists */
    ptr->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    ptr->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    ptr->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);


    /* Populate the initialized lists */
    populateLists(ptr->waypoints, ptr->routes, ptr->tracks, root_element->children);

    /* cleanup any alloced memory */
    xmlFreeDoc(doc);

    return ptr;

}

void populateLists(List *waypoints, List *routes, List *tracks, xmlNode *root_element){

    if(waypoints == NULL || routes == NULL || tracks == NULL){
        return;
    }

    xmlNode *cur_node = NULL;

    for (cur_node = root_element; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            // if the name is "wpt"
            if(strcmp((char *)cur_node->name, "wpt")==0){

                // Add the waypoint to the list of waypoints
                Waypoint *wpt = createWaypoint(cur_node);
                insertBack(waypoints, (void *) wpt);

            }
            
            // if the name is "rte"
            if(strcmp((char *)cur_node->name, "rte")==0){
                
                // Add a route to the list of routes
                Route *rte = createRoute(cur_node);
                insertBack(routes, (void *) rte);

            }

            // if the name is "trk"
            if(strcmp((char *)cur_node->name, "trk")==0){
                
                // Add a track to the list of tracks
                Track *trk = createTrack(cur_node);
                insertBack(tracks, (void *) trk);

            }
        }
    }
}

char* GPXdocToString(GPXdoc* doc){

    if(doc == NULL){
        return NULL;
    }

    /* initialize the length of the header */
    int header_titles = 40;
    int len_header = strlen(doc->namespace) + strlen(doc->creator) + 325 + header_titles;
    
    /* Make header string */
    char *header = malloc(sizeof(char)*len_header);
    sprintf(header, "Namespace: %s\nCreator: %s\nVersion: %lf\n\n", doc->namespace, doc->creator, doc->version);

    
    /* get the string representations of each list in the doc */
    char *waypoints = toString(doc->waypoints);
    char *routes = toString(doc->routes);
    char *tracks = toString(doc->tracks);

    /* initialize the length of the items in the doc */
    int item_titles = 40;
    int len_items = strlen(waypoints) + strlen(routes) + strlen(tracks) + item_titles;

    /* Make item string */
    char *items = malloc(sizeof(char)*len_items);
    sprintf(items, "Waypoints:%s\nRoutes:%s\nTracks:%s", waypoints, routes, tracks);

    /* create the string for the output */
    int len = len_header + len_items;
    char *string = malloc(sizeof(char)*len);
    strcpy(string, header);
    strcat(string, items);

    /* free all temp strings */
    free(waypoints);
    free(routes);
    free(tracks);

    free(header);
    free(items);

    return string;
}

void deleteGPXdoc(GPXdoc* doc){

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
    
    if(doc == NULL){
        return;
    }

    free(doc->creator);
    freeList(doc->waypoints);
    freeList(doc->routes);
    freeList(doc->tracks);
    free(doc);
}

int getNumWaypoints(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->waypoints);
}

int getNumRoutes(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->routes);
}

int getNumTracks(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->tracks);
}

int getNumSegments(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }
    
    int sum = 0;
    List *tracks = doc->tracks;
    ListIterator li = createIterator(tracks);
    void *elem;

    /* iterate through the list of tracks and sum the segments each one has */
    while((elem = nextElement(&li)) != NULL){
        Track *track = (Track*) elem;
        sum += getLength(track->segments);
    }

    return sum;
}


int getNumGPXData(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }
    
    int sum = 0;
    List *list;
    ListIterator li;

    /* Waypoints GPXData */
    list = doc->waypoints;
    li = createIterator(list);

    void *wpt_elem1 = nextElement(&li);
    while(wpt_elem1 != NULL){
        Waypoint *wpt1 = (Waypoint*)wpt_elem1;
        
        // check name, if not empty, sum += 1
        if(strcmp(wpt1->name, "")!= 0 && wpt1->name != NULL){
            sum++;
        }

        // sum += length of otherdata
        sum += wpt1->otherData->length;

        wpt_elem1 = nextElement(&li);
    }
    
    /* Routes GPXData */
    list = doc->routes;
    li = createIterator(list);
    
    void *rte_elem = nextElement(&li);
    while(rte_elem != NULL){
        
        Route *rte = (Route*)rte_elem;
        
        // check name, if not empty, sum += 1
        if(strcmp(rte->name, "")!= 0 && rte->name != NULL){
            sum++;
        }

        // check waypoints
        ListIterator wpt_li1 = createIterator(rte->waypoints);
        void *wpt_elem2 = nextElement(&wpt_li1);
        while(wpt_elem2 != NULL){

            Waypoint *wpt2 = (Waypoint*)wpt_elem2;
            
            // check name, if not empty, sum += 1
            if(strcmp(wpt2->name, "")!= 0 && wpt2->name != NULL){
                sum++;
            }
            
            sum += wpt2->otherData->length;
            wpt_elem2 = nextElement(&wpt_li1);
        }

        // sum += length otherdata
        sum += rte->otherData->length;

        rte_elem = nextElement(&li);
    }

    /* Tracks GPXData */
    list = doc->tracks;
    li = createIterator(list);
    
    void *trk_elem = nextElement(&li);
    while(trk_elem != NULL){

        Track *trk = (Track*)trk_elem;
        
        // check name, if not empty, sum += 1
        if(strcmp(trk->name, "")!= 0 && trk->name != NULL){
            sum++;
        }

        // check segments
        ListIterator segment_li = createIterator(trk->segments);
        void *segment_elem = nextElement(&segment_li);
        while(segment_elem != NULL){
            TrackSegment *segment = (TrackSegment*)segment_elem;

            //check waypoints
            ListIterator wpt_li2 = createIterator(segment->waypoints);
            void *wpt_elem3 = nextElement(&wpt_li2);
            while(wpt_elem3 != NULL){

                Waypoint *wpt3 = (Waypoint*)wpt_elem3;

                // check name, if not empty, sum += 1
                if(strcmp(wpt3->name, "")!= 0 && wpt3->name != NULL){
                    sum++;
                }

                // sum += length otherdata
                sum += wpt3->otherData->length;

                wpt_elem3 = nextElement(&wpt_li2);
            }
            segment_elem = nextElement(&segment_li);
        }

        // sum += length otherdata
        sum += trk->otherData->length;
        trk_elem = nextElement(&li);
    }

    return sum;
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name){
    
    if(doc == NULL || name == NULL){
        return NULL;
    }

    List *waypoints = doc->waypoints;
    Waypoint *wpt = malloc(sizeof(Waypoint));
    wpt->name = malloc(strlen(name) + 1);
    strcpy(wpt->name, name);

	ListIterator itr = createIterator(waypoints);

	void* data;

	while ((data = nextElement(&itr)) != NULL)
	{
		if (compareWaypoints(data, wpt) == 0){
            free(wpt->name);
            free(wpt);
            return data;
        } 
	}

    free(wpt->name);
    free(wpt);

    return NULL;
}

Track* getTrack(const GPXdoc* doc, char* name){

    if(doc == NULL || name == NULL){
        return NULL;
    }

    List *tracks = doc->tracks;
    Track *trk = malloc(sizeof(Track));
    trk->name = malloc(strlen(name)+1);
    strcpy(trk->name, name);

    ListIterator itr = createIterator(tracks);

	void* data;

	while ((data = nextElement(&itr)) != NULL)
	{
		if (compareTracks(data, trk) == 0){
            free(trk->name);
            free(trk);
            return data;  
        } 
	}

    free(trk->name);
    free(trk);

    return NULL;
}

Route* getRoute(const GPXdoc* doc, char* name){

    if(doc == NULL || name == NULL){
        return NULL;
    }

    List *routes = doc->routes;
    Route *rte = malloc(sizeof(Route));
    rte->name = malloc(strlen(name)+1);
    strcpy(rte->name, name);

    ListIterator itr = createIterator(routes);

	void* data;

	while ((data = nextElement(&itr)) != NULL)
	{
		if (compareTracks(data, rte) == 0){
            free(rte->name);
            free(rte);
            return data;  
        } 
	}

    free(rte->name);
    free(rte);

    return NULL;
}

void deleteGpxData( void* data){
    
    if (data == NULL){
        return;
    }
    
    free(data);
}
char* gpxDataToString( void* data){

    if (data == NULL){
        return NULL;
    }

    /* (void *) is assumed to be of type GPXData */
    GPXData *gpxd = (GPXData*) data;

    int num_extra_chars = 20;
    int len = strlen(gpxd->name) + strlen(gpxd->value) + num_extra_chars;
    char *string = (char*)malloc(sizeof(char)*len);

    sprintf(string, "%s: %s\n", gpxd->name, gpxd->value);
    
    return string;
}
int compareGpxData(const void *first, const void *second){
    
    if(first == NULL || second == NULL){
        return 0;
    }

    GPXData *other1 = (GPXData*)first;
    GPXData *other2 = (GPXData*)second;

    return strcmp((char *)other1->name, (char *)other2->name);
}

void deleteWaypoint(void* data){
    
    if (data == NULL){
        return;
    }

    Waypoint *wpt = (Waypoint*) data;
    free(wpt->name);
    freeList(wpt->otherData);
    free(wpt);
}
char* waypointToString( void* data){
    
    if (data == NULL){
        return NULL;
    }

    Waypoint *wpt = (Waypoint*) data;

    char *otherData = toString(wpt->otherData);

    int num_extra_chars = 80;
    int len = strlen(wpt->name) + strlen(otherData) + num_extra_chars;
    char *string = (char*)malloc(sizeof(char)*len);

    sprintf(string, "%s\nLatitude: %lf, Longitude: %lf%s", wpt->name, wpt->latitude, wpt->longitude, otherData);
    
    free(otherData);
    
    return string;
}
int compareWaypoints(const void *first, const void *second){
    
    if(first == NULL || second == NULL){
        return 0;
    }

    Waypoint *wpt1 = (Waypoint*)first;
    Waypoint *wpt2 = (Waypoint*)second;

    return strcmp((char *)wpt1->name, (char *)wpt2->name);
}

void deleteRoute(void* data){

    if (data == NULL){
        return;
    }

    Route *rte = (Route*) data;
    free(rte->name);
    freeList(rte->waypoints);
    freeList(rte->otherData);
    free(rte);
}
char* routeToString(void* data){

    if (data == NULL){
        return NULL;
    }

    Route *rte = (Route*) data;
    char *wpts = toString(rte->waypoints);
    char *otherData = toString(rte->otherData);

    int num_extra_chars = 40;
    int len = strlen(rte->name) + strlen(wpts) + strlen(otherData) + num_extra_chars;
    char *string = (char*)malloc(sizeof(char)*len);

    sprintf(string, "Route %s with points:%s\nAdditionally: %s", rte->name, wpts, otherData);
    
    free(wpts);
    free(otherData);

    return string;
}
int compareRoutes(const void *first, const void *second){
    if(first == NULL || second == NULL){
        return 0;
    }

    Route *rte1 = (Route*)first;
    Route *rte2 = (Route*)second;

    return strcmp((char *)rte1->name, (char *)rte2->name);
}

void deleteTrackSegment(void* data){
    if (data == NULL){
        return;
    }

    TrackSegment *segment = (TrackSegment*) data;
    freeList(segment->waypoints);
    free(data);
}
char* trackSegmentToString(void* data){
    if (data == NULL){
        return NULL;
    }

    TrackSegment *segment = (TrackSegment*) data;
    char *wpts = toString(segment->waypoints);

    int num_extra_chars = 100;
    int len = strlen(wpts) + num_extra_chars;
    char *string = (char*)malloc(sizeof(char)*len);

    sprintf(string, "Segment with Track Points:%s", wpts);
    
    free(wpts);

    return string;
}
int compareTrackSegments(const void *first, const void *second){

    if(first == NULL || second == NULL){
        return 0;
    }

    TrackSegment *segment1 = (TrackSegment*)first;
    TrackSegment *segment2 = (TrackSegment*)second;

    double coord1 = 0;
    double coord2 = 0;

    ListIterator li = createIterator(segment1->waypoints);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Waypoint *wpt1 = (Waypoint*)elem;
        coord1 += wpt1->longitude + wpt1->latitude;
        elem = nextElement(&li);
    }

    li = createIterator(segment2->waypoints);
    elem = nextElement(&li);
    while(elem != NULL){
        Waypoint *wpt2 = (Waypoint*)elem;
        coord2 += wpt2->longitude + wpt2->latitude;
        elem = nextElement(&li);
    }

    if(coord1 == coord2){
        return 0;
    }

    return -1;
}

void deleteTrack(void* data){
    
    if(data == NULL){
        return;
    }

    Track *track = (Track*)data; 
    free(track->name);
    freeList(track->segments);
    freeList(track->otherData);
    free(track);

}
char* trackToString(void* data){

    if (data == NULL){
        return NULL;
    }

    Track *trk = (Track*) data;
    char *segments = toString(trk->segments);
    char *otherData = toString(trk->otherData);

    int num_extra_chars = 50;
    int len = strlen(trk->name) + strlen(segments) + strlen(otherData) + num_extra_chars;
    char *string = (char*)malloc(sizeof(char)*len);

    sprintf(string, "%s with segments:%s\nAdditionally: %s", trk->name, segments, otherData);
    
    free(segments);
    free(otherData);

    return string;
}
int compareTracks(const void *first, const void *second){
    if(first == NULL || second == NULL){
        return 0;
    }

    Track *trk1 = (Track*)first;
    Track *trk2 = (Track*)second;

    return strcmp((char *)trk1->name, (char *)trk2->name);
}

GPXdoc *createValidGPXdoc(char *fileName, char *gpxSchemaFile){

    if (fileName == NULL || gpxSchemaFile == NULL){
        return NULL;
    }

    xmlSchemaParserCtxtPtr parserCtxt = NULL;
    xmlSchemaPtr schema = NULL;
    xmlSchemaValidCtxtPtr validCtxt = NULL;

    parserCtxt = xmlSchemaNewParserCtxt(gpxSchemaFile);
    if (parserCtxt == NULL){
        return NULL;
    }

    schema = xmlSchemaParse(parserCtxt);
    if (schema == NULL){
        xmlSchemaFreeParserCtxt(parserCtxt);
        return NULL;
    }

    validCtxt = xmlSchemaNewValidCtxt(schema);
    if(!validCtxt) {
        xmlSchemaFreeParserCtxt(parserCtxt);
        xmlSchemaFree(schema);
        return NULL;
    }
    
    if(xmlSchemaValidateFile(validCtxt, fileName, 0) != 0){
        xmlSchemaFreeParserCtxt(parserCtxt);
        xmlSchemaFree(schema);
        xmlSchemaFreeValidCtxt(validCtxt);
        return NULL;
    }

    GPXdoc *doc = createGPXdoc(fileName);
    return doc;
}

bool writeGPXdoc(GPXdoc *doc, char *fileName){

    if(doc == NULL || fileName == NULL){
        return false;
    }
    xmlDocPtr xml = GPXToXML(doc);
    if (xml == NULL){
        return false;
    }
    xmlSaveFormatFileEnc(fileName, xml, "ISO-8859-1", 1);
    xmlFreeDoc(xml);
    xmlMemoryDump();

    return true;
}

bool validateGPXDoc(GPXdoc *gpxDoc, char *gpxSchemaFile){

    if (gpxDoc == NULL || gpxSchemaFile == NULL){
        return false;
    }

    xmlDocPtr doc = NULL;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    doc = GPXToXML(gpxDoc);

    if(doc == NULL){
        return false;
    } else {
        xmlSchemaValidCtxtPtr ctxt;
    
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
        ret = xmlSchemaValidateDoc(ctxt, doc);

        if(ret != 0){
            return false;
        }

        xmlSchemaFreeValidCtxt(ctxt);
        xmlFreeDoc(doc);
    }
    
    if(schema != NULL){
        xmlSchemaFree(schema);
    }

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();

    return true;
}

float round10(float len){
    float remainder = fmod(len, 10);
    if(remainder < 5){
        return len - remainder;
    }else{
        return len + (10 - remainder);
    }
}

float getRouteLen(const Route *rt){

    if (rt == NULL){
        return 0;
    }
    
    float total = 0;

    ListIterator li = createIterator(rt->waypoints);
    void *elem1 = nextElement(&li);
    void *elem2 = nextElement(&li);
    
    while(elem2 != NULL){
        Waypoint *pt1 = (Waypoint *)elem1;
        Waypoint *pt2 = (Waypoint *)elem2;

        total += distanceBetweenPoints(pt1, pt2);

        elem1 = elem2;
        elem2 = nextElement(&li);
    }

    return total;
}

float getTrackLen(const Track *tr){

    if (tr == NULL){
        return 0;
    }
    
    float total = 0;

    Waypoint *first;
    Waypoint *last;

    int has_run_once = 0;

    ListIterator li = createIterator(tr->segments);
    void *elem = nextElement(&li);

    while(elem != NULL){

        TrackSegment *seg = (TrackSegment*)elem;
        ListIterator li2 = createIterator(seg->waypoints);
        void *elem1 = nextElement(&li2);
        void *elem2 = nextElement(&li2);

        if (has_run_once == 1 && elem1 != NULL){
            first = (Waypoint *)elem1;
            total += distanceBetweenPoints(first, last);
        }
        
        while(elem2 != NULL){
            Waypoint *pt1 = (Waypoint *)elem1;
            Waypoint *pt2 = (Waypoint *)elem2;

            total += distanceBetweenPoints(pt1, pt2);

            last = pt2;
            elem1 = elem2;
            elem2 = nextElement(&li2);
            has_run_once = 1;
        }
        elem = nextElement(&li);
    }

    return total;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta){
    
    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }
    
    int total = 0;
    float routelen;

    ListIterator li = createIterator(doc->routes);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Route *rt = (Route *)elem;
        routelen = getRouteLen(rt);
        
        if((routelen < (len + delta)) && (routelen > (len - delta))){
            total += 1;
        }
        
        elem = nextElement(&li);
    }

    return total;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta){
    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }
    
    int total = 0;
    float tracklen;

    ListIterator li = createIterator(doc->tracks);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Track *tr = (Track *)elem;
        tracklen = getTrackLen(tr);
        
        if((tracklen < (len + delta)) && (tracklen > (len - delta))){
            total += 1;
        }
        
        elem = nextElement(&li);
    }

    return total;
}

bool isLoopRoute(const Route* route, float delta){

    if(route == NULL || delta < 0){
        return false;
    }

    if(route->waypoints->length < 4){
        return false;
    }

    Waypoint *first = (Waypoint *)getFromFront(route->waypoints);
    Waypoint *last = (Waypoint *)getFromBack(route->waypoints);

    if(distanceBetweenPoints(first, last) < delta){
        return true;
    }

    return false;

}

bool isLoopTrack(const Track *tr, float delta){

    if(tr == NULL || delta < 0){
        return false;
    }

    ListIterator li = createIterator(tr->segments);
    void *elem = nextElement(&li);
    while(elem != NULL){
        TrackSegment *seg = (TrackSegment *)elem;
        if(seg->waypoints->length > 3){
            break;
        }
        elem = nextElement(&li);
    }

    if(elem == NULL){
        return false;
    }
    
    TrackSegment *firstseg = (TrackSegment *)getFromFront(tr->segments);
    Waypoint *first = (Waypoint *)getFromFront(firstseg->waypoints);
    TrackSegment *lastseg = (TrackSegment *)getFromBack(tr->segments);
    Waypoint *last = (Waypoint *)getFromBack(lastseg->waypoints);

    if(distanceBetweenPoints(first, last) < delta){
        return true;
    }

    return false;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    if(doc == NULL){
        return NULL;
    }  

    List *routesBetween = initializeList(&routeToString, &dummyDelete, &compareRoutes);
    Waypoint *source = (Waypoint *)malloc(sizeof(Waypoint));
    source->latitude = sourceLat;
    source->longitude = sourceLong;
    Waypoint *dest = (Waypoint *)malloc(sizeof(Waypoint));
    dest->latitude = destLat;
    dest->longitude = destLong;

    ListIterator li = createIterator(doc->routes);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Route *rt = (Route *)elem;
        Waypoint *first = getFromFront(rt->waypoints);
        Waypoint *last = getFromBack(rt->waypoints);

        float distanceFirst = distanceBetweenPoints(source, first);
        float distanceLast = distanceBetweenPoints(dest, last);

        if (distanceFirst < delta && distanceLast < delta){
            insertBack(routesBetween, (void *)rt);
        }

        elem = nextElement(&li);
    }

    if(routesBetween->length == 0){
        freeList(routesBetween);
        free(source);
        free(dest);
        return NULL;
    }
    
    free(source);
    free(dest);

    return routesBetween;
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    
    if(doc == NULL){
        return NULL;
    }  

    List *tracksBetween = initializeList(&trackToString, &dummyDelete, &compareRoutes);
    Waypoint *source = (Waypoint *)malloc(sizeof(Waypoint));
    source->latitude = sourceLat;
    source->longitude = sourceLong;
    Waypoint *dest = (Waypoint *)malloc(sizeof(Waypoint));
    dest->latitude = destLat;
    dest->longitude = destLong;

    ListIterator li = createIterator(doc->tracks);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Track *track = (Track *)elem;
        
        TrackSegment *firstseg = (TrackSegment *)getFromFront(track->segments);
        Waypoint *first = (Waypoint *)getFromFront(firstseg->waypoints);
        TrackSegment *lastseg = (TrackSegment *)getFromBack(track->segments);
        Waypoint *last = (Waypoint *)getFromBack(lastseg->waypoints);

        float distanceFirst = distanceBetweenPoints(source, first);
        float distanceLast = distanceBetweenPoints(dest, last);

        if (distanceFirst < delta && distanceLast < delta){
            insertBack(tracksBetween, (void *)track);
        }

        elem = nextElement(&li);
    }

    if(tracksBetween->length == 0){
        freeList(tracksBetween);
        free(source);
        free(dest);
        return NULL;
    }
    
    free(source);
    free(dest);

    return tracksBetween;
}

char* routeListToJSON(const List *list){

    if (list == NULL){
        char *JSON = (char *)malloc(3);
        strcpy(JSON, "[]");
        return JSON;
    }

    int size = 3;
    char *JSON = (char *) malloc(size);

    strcpy(JSON, "[");

    ListIterator li = createIterator((List *) list);
    void *elem = nextElement(&li);

    while(elem != NULL){
        Route *rt = (Route *)elem;
        JSON = addRouteJSON(rt, JSON, &size);
        elem = nextElement(&li);
        if(elem != NULL){
            strcat(JSON, ",");
        }
    }
    strcat(JSON, "]");

    return JSON;
}

char* trackListToJSON(const List *list){
    if (list == NULL){
        char *JSON = (char *)malloc(3);
        strcpy(JSON, "[]");
        return JSON;
    }

    int size = 3;
    char *JSON = (char *) malloc(size);

    strcpy(JSON, "[");

    ListIterator li = createIterator((List *) list);
    void *elem = nextElement(&li);

    while(elem != NULL){
        Track *tr = (Track *)elem;
        JSON = addTrackJSON(tr, JSON, &size);
        elem = nextElement(&li);
        if(elem != NULL){
            strcat(JSON, ",");
        }
    }
    strcat(JSON, "]");

    return JSON;
}

char* trackToJSON(const Track *tr){

    char *JSON = NULL;

    if(tr == NULL){
        JSON = (char *)malloc(3);
        strcpy(JSON, "{}");
        return JSON;
    }

    char none[5] = "None";
    char loopStat[6];
    strcpy(loopStat, isLoopTrack(tr, 10) == true ? "true" : "false");

    if(strcmp(tr->name, "") == 0){
        JSON = (char *)malloc(5 + 10 + 100 + 6 + 1 + 100);
        sprintf(JSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.0f,\"loop\":%s}", none, getNumTrackPts(tr), round10(getTrackLen(tr)), loopStat);
    } else {
        JSON = (char *)malloc(strlen(tr->name) + 10 + 100 + 6 + 1 + 100);
        sprintf(JSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.0f,\"loop\":%s}", tr->name, getNumTrackPts(tr), round10(getTrackLen(tr)), loopStat);
    }

    return JSON;    
}

char* routeToJSON(const Route *rt){

    char *JSON = NULL;

    if(rt == NULL){
        JSON = (char *)malloc(3);
        strcpy(JSON, "{}");
        return JSON;
    }

    char none[5] = "None";

    char loopStat[6];
    strcpy(loopStat, (isLoopRoute(rt, 10) == true) ? "true" : "false");

    if(strcmp(rt->name, "") == 0){
        JSON = (char *)malloc(5 + 10 + 100 + 6 + 1 + 100);
        sprintf(JSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.0f,\"loop\":%s}", none, rt->waypoints->length, round10(getRouteLen(rt)), loopStat);
    } else {
        JSON = (char *)malloc(strlen(rt->name) + 10 + 100 + 6 + 1 + 100);
        sprintf(JSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.0f,\"loop\":%s}", rt->name, rt->waypoints->length, round10(getRouteLen(rt)), loopStat);
    }

    return JSON;
}

char* GPXtoJSON(const GPXdoc* gpx){
    if(gpx == NULL){
        char *JSON = (char *)malloc(3);
        strcpy(JSON, "{}");
        return JSON;
    }
    
    int size = 10 + strlen(gpx->creator) + 30 + 100;
    char *JSON = (char *)malloc(size);
    sprintf(JSON, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, gpx->waypoints->length, gpx->routes->length, gpx->tracks->length);

    return JSON;
}

void addWaypoint(Route *rt, Waypoint *pt){
    if (rt == NULL || pt == NULL){
        return;
    }

    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt){
    if (rt == NULL || doc == NULL){
        return;
    }

    insertBack(doc->routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString){
    
    if (gpxString == NULL){
        return NULL;
    }

    GPXdoc *doc = (GPXdoc *)malloc(sizeof(GPXdoc));
    char version[5];
    char creator[256];
    doc->creator = (char *) malloc(strlen(gpxString));

    sscanf(gpxString, "{\"version\":%[0-9.],\"creator\":\"%[a-zA-Z ]\"}", version, creator);

    doc->version = atof(version);
    strcpy(doc->creator, creator);
    strcpy(doc->namespace, "http://www.topografix.com/GPX/1/1");
    
    /* Initialize the lists */
    doc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    doc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    doc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    return doc;

}

Waypoint* JSONtoWaypoint(const char* gpxString){
    
    if (gpxString == NULL){
        return NULL;
    }
    
    Waypoint *wpt = (Waypoint *)malloc(sizeof(Waypoint));
    char lat[MAX_BUF];
    char lon[MAX_BUF];

    sscanf(gpxString, "{\"lat\":%[0-9.-],\"lon\":%[0-9.-]}", lat, lon);

    wpt->latitude = atof(lat);
    wpt->longitude = atof(lon);
    wpt->name = (char *)malloc(3);
    strcpy(wpt->name, "");
    
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    return wpt;
}

Route* JSONtoRoute(const char* gpxString){
    if (gpxString == NULL){
        return NULL;
    }
    
    Route *rte = (Route *)malloc(sizeof(Route));
    char name[MAX_BUF];

    sscanf(gpxString, "{\"name\":\"%[a-zA-Z ,'-]\"}", name);
    rte->name = (char *) malloc(strlen(name));
    strcpy(rte->name, name);
    
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    return rte;
}