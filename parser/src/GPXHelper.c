#include "GPXParser.h"
#include "LinkedListAPI.h"
#include "GPXHelper.h"

GPXData *createGPXData(xmlNode *node){

    if(node == NULL){
        return NULL;
    }

    /* set name */
    char *name;
    
    if(node->name != NULL){
        name = (char *)node->name;
    } else {
        name = "";
    }

    /* set value */
    char *value = (char *) xmlNodeGetContent(node);
    
    GPXData *data = malloc(sizeof(GPXData) + strlen(name) + strlen(value));
    strcpy(data->name, name);
    strcpy(data->value, value);
    free(value);

    return data;
}

Waypoint *createWaypoint(xmlNode *node){
    
    if(node == NULL){
        return NULL;
    }
    
    // Create an uninitialized Waypoint
    Waypoint *wpt = malloc(sizeof(Waypoint));
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    xmlNode *wpt_child = node->children;
    wpt->name = NULL;

    while(wpt_child != NULL){
        if (strcmp((char*)wpt_child->name,"text") != 0){
            if (strcmp((char*)wpt_child->name, "name") == 0){
                if(node->name != NULL){
                    wpt->name = (char*)xmlNodeGetContent(wpt_child);
                }
            }else{
                GPXData *data = createGPXData(wpt_child);
                insertBack(wpt->otherData, (void*)data);
            }
        }
        wpt_child = wpt_child->next;
    }

    // set it's attributes
    xmlAttr *attr;
    for (attr = node->properties; attr != NULL; attr = attr->next)
    {
        // Longitude
        if(strcmp((char *)attr->name, "lon") == 0){
            char *cont = (char *)(attr->children->content);
            double lon = atof(cont);
            wpt->longitude = lon;
        }
        // Latitude
        if(strcmp((char *)attr->name, "lat") == 0){
            char *cont = (char *)(attr->children->content);
            double lat = atof(cont);
            wpt->latitude = lat;
        } 
    }

    if(wpt->name == NULL){
        wpt->name = malloc(sizeof(char)*2);
        strcpy(wpt->name, "");
    }

    return wpt;
}

Route *createRoute(xmlNode *node){

    if(node == NULL){
        return NULL;
    }

    // Create a Route
    Route *rte = malloc(sizeof(Route));
    xmlNode *rte_child = node->children;
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rte->name = NULL;

    // Loop through all the nodes and add waypoints
    while(rte_child != NULL){
        if (strcmp((char*)rte_child->name,"text") != 0){
            if(strcmp((char*)rte_child->name, "rtept") == 0){
                Waypoint *wpt = createWaypoint(rte_child);
                insertBack(rte->waypoints, (void*)wpt);
            }else if (strcmp((char*)rte_child->name, "name") == 0){
                if(node->name != NULL){
                    // char *name = (char *)xmlNodeGetContent(rte_child);
                    // rte->name = realloc(rte->name, strlen(name));
                    // strcpy(rte->name, name);
                    // free(name);
                    rte->name = (char*)xmlNodeGetContent(rte_child);
                }
            }else{
                GPXData *data = createGPXData(rte_child);
                insertBack(rte->otherData, (void*)data);
            }
        }
        rte_child = rte_child->next;
    }

    if(rte->name == NULL){
        rte->name = malloc(sizeof(char)*2);
        strcpy(rte->name, "");
    }

    return rte;
}

TrackSegment *createTrackSegment(xmlNode *node){

    if(node == NULL){
        return NULL;
    }

    // Create a Route
    TrackSegment *segment = malloc(sizeof(TrackSegment));
    segment->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    xmlNode *segment_child = node->children;

    // Loop through all the nodes and add waypoints
    while(segment_child != NULL){
        if (strcmp((char*)segment_child->name,"text") != 0){
            if(strcmp((char*)segment_child->name, "trkpt") == 0){
                Waypoint *wpt = createWaypoint(segment_child);
                insertBack(segment->waypoints, (void*)wpt);
            }
        }
        segment_child = segment_child->next;
    }

    return segment;
}

Track *createTrack(xmlNode *node){

    if(node == NULL){
        return NULL;
    }

    // Create Track
    Track *trk = malloc(sizeof(Track));
    xmlNode *trk_child = node->children;
    trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    trk->name = NULL;

    // Loop through to find and set all segments
    while (trk_child != NULL){
        if (strcmp((char*)trk_child->name,"text") != 0){
            if(strcmp((char*)trk_child->name, "trkseg") == 0){

                TrackSegment *segment = createTrackSegment(trk_child);
                insertBack(trk->segments, (void*)segment);

            }else if(strcmp((char*)trk_child->name, "name") == 0){
                
                if(node->name != NULL){
                    // char *name = (char *)xmlNodeGetContent(trk_child);
                    // trk->name = realloc(trk->name, strlen(name));
                    // strcpy(trk->name, name);
                    // free(name);
                    trk->name = (char*)xmlNodeGetContent(trk_child);
                }
                
            }else{
                GPXData *data = createGPXData(trk_child);
                insertBack(trk->otherData, (void*)data);
            }
        }
        trk_child = trk_child->next;
    }

    if(trk->name == NULL){
        trk->name = malloc(sizeof(char)*2);
        strcpy(trk->name, "");
    }

    return trk;

}

void createXmlOtherNode(GPXData *data, xmlNodePtr parent, xmlNsPtr ns){
    xmlNewChild(parent, ns, (xmlChar *) data->name, (xmlChar *) data->value);
}

void createXmlPtNode(Waypoint *wpt, xmlNodePtr parent, xmlNsPtr ns, char *wptType){
    
    char lat[MAX_BUF];
    sprintf(lat, "%f", wpt->latitude);
    char lon[MAX_BUF];
    sprintf(lon, "%f", wpt->longitude);

    xmlNodePtr wptNode = xmlNewChild(parent, ns, (xmlChar *) wptType, NULL);
    
    xmlNewProp(wptNode, (xmlChar *) "lat", (xmlChar *) lat);
    xmlNewProp(wptNode, (xmlChar *) "lon", (xmlChar *) lon);
    
    if(strcmp(wpt->name, "") != 0){
        xmlNewChild(wptNode, ns, (xmlChar *) "name", (xmlChar *) wpt->name);
    }

    ListIterator li = createIterator(wpt->otherData);
    void *elem = nextElement(&li);
    while(elem != NULL){
        GPXData *gpxData = (GPXData*) elem;
        createXmlOtherNode(gpxData, wptNode, ns);
        elem = nextElement(&li);
    }
}
void createXmlRteNode(Route *rte, xmlNodePtr parent, xmlNsPtr ns){

    xmlNodePtr rteNode = xmlNewChild(parent, ns, (xmlChar *) "rte", NULL);
    
    if(strcmp(rte->name, "") != 0){
        xmlNewChild(rteNode, ns, (xmlChar *) "name", (xmlChar *) rte->name);
    }

    ListIterator li = createIterator(rte->otherData);
    void *elem = nextElement(&li);
    while(elem != NULL){
        GPXData *gpxData = (GPXData*) elem;
        createXmlOtherNode(gpxData, rteNode, ns);
        elem = nextElement(&li);
    }

    li = createIterator(rte->waypoints);
    elem = nextElement(&li);
    while(elem != NULL){
        Waypoint *wpt = (Waypoint*) elem;
        createXmlPtNode(wpt, rteNode, ns, "rtept");
        elem = nextElement(&li);
    }
}

void createXmlSegNode(TrackSegment *seg, xmlNodePtr parent, xmlNsPtr ns){
    xmlNodePtr segNode = xmlNewChild(parent, ns, (xmlChar *) "trkseg", NULL);

    ListIterator li = createIterator(seg->waypoints);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Waypoint *wpt = (Waypoint*) elem;
        createXmlPtNode(wpt, segNode, ns, "trkpt");
        elem = nextElement(&li);
    }
}

void createXmlTrkNode(Track *trk, xmlNodePtr parent, xmlNsPtr ns){
    xmlNodePtr trkNode = xmlNewChild(parent, ns, (xmlChar *) "trk", NULL);
    
    if(strcmp(trk->name, "") != 0){
        xmlNewChild(trkNode, ns, (xmlChar *) "name", (xmlChar *) trk->name);
    }

    ListIterator li = createIterator(trk->otherData);
    void *elem = nextElement(&li);
    while(elem != NULL){
        GPXData *gpxData = (GPXData*) elem;
        createXmlOtherNode(gpxData, trkNode, ns);
        elem = nextElement(&li);
    }

    li = createIterator(trk->segments);
    elem = nextElement(&li);
    while(elem != NULL){
        TrackSegment *seg = (TrackSegment*) elem;
        createXmlSegNode(seg, trkNode, ns);
        elem = nextElement(&li);
    }
}

xmlDocPtr GPXToXML(GPXdoc *gpxDoc){

    if(gpxDoc == NULL || gpxDoc->creator == NULL || gpxDoc->namespace == NULL || gpxDoc->waypoints == NULL || gpxDoc->routes == NULL || gpxDoc->tracks == NULL){
        return NULL;
    }
    
    /* initialize empty pointers */
    xmlDocPtr xmlDoc = NULL;
    xmlNodePtr root_node = NULL;

    LIBXML_TEST_VERSION;

    /* create a blank document with gpx as the root node */
    xmlDoc = xmlNewDoc((xmlChar *) "1.0");
    root_node = xmlNewNode(NULL, (xmlChar *) "gpx");
    xmlNsPtr ns = xmlNewNs(root_node, (xmlChar *) gpxDoc->namespace, NULL);
    xmlSetNs(root_node, ns);
    xmlDocSetRootElement(xmlDoc, root_node);

    /* Set the version and the creator */
    char version[MAX_BUF];
    sprintf(version, "%.1f", gpxDoc->version);
    xmlNewProp(root_node, (xmlChar *) "version", (xmlChar *)version);
    xmlNewProp(root_node, (xmlChar *) "creator", (xmlChar *)gpxDoc->creator);

    ListIterator li = createIterator(gpxDoc->waypoints);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Waypoint *wpt = (Waypoint *)elem;
        createXmlPtNode(wpt, root_node, ns, "wpt");
        elem = nextElement(&li);
    }
    
    li = createIterator(gpxDoc->routes);
    elem = nextElement(&li);
    while(elem != NULL){
        Route *rte = (Route *)elem;
        createXmlRteNode(rte, root_node, ns);
        elem = nextElement(&li);
    }
    
    li = createIterator(gpxDoc->tracks);
    elem = nextElement(&li);
    while(elem != NULL){
        Track *trk = (Track *)elem;
        createXmlTrkNode(trk, root_node, ns);
        elem = nextElement(&li);
    }

    return xmlDoc;
}

float distanceBetweenPoints(Waypoint *pt1, Waypoint *pt2){
    double phi1 = pt1->latitude * (M_PI/180);
    double phi2 = pt2->latitude * (M_PI/180);

    double delta_phi = (pt2->latitude - pt1->latitude) * (M_PI/180);
    double delta_lambda = (pt2->longitude - pt1->longitude) * (M_PI/180);

    double a = sin(delta_phi/2) * sin(delta_phi/2) + cos(phi1) * cos(phi2) * sin(delta_lambda/2) * sin(delta_lambda/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return EARTHR * c;
}

void dummyDelete(void *data){
    return;
}

char *addRouteJSON(Route *rt, char *string, int *size){
    char *rtJSONstring = routeToJSON(rt);
    (*size) += strlen(rtJSONstring) + 100;
    char *output = (char *)realloc(string, (*size));
    strcat(output, rtJSONstring);
    free(rtJSONstring);
    return output;
}

char *addTrackJSON(Track *tr, char *string, int *size){
    char *trJSONstring = trackToJSON(tr);
    (*size) += strlen(trJSONstring) + 100;
    char *output = (char *)realloc(string, (*size));
    strcat(output, trJSONstring);
    free(trJSONstring);
    return output;
}

char *fileToJSON(char *filename){
    GPXdoc *doc = createGPXdoc(filename);
    char *string = GPXtoJSON(doc);
    deleteGPXdoc(doc);
    return string;
}

char *getRoutesComponent(char *filename)
{
    GPXdoc *doc = createGPXdoc(filename);
    
    char *string = (char *)malloc(5);
    strcpy(string, "[");

    ListIterator li = createIterator(doc->routes);
    void *elem = nextElement(&li);
    while(elem != NULL)
    {
        Route *rt = (Route *)elem;
        string = addRouteAndOD(string, rt);
        elem = nextElement(&li);
        if (elem != NULL){
            strcat(string, ",");
        }
    }
    strcat(string, "]");

    deleteGPXdoc(doc);
    return string;
}

char *addRouteAndOD(char *string, Route *rt)
{
    char *routeJSON = routeToJSON(rt);
    char *otherDataJSON = GPXDataListToJSON(rt->otherData);
    
    char *fullJSON = (char *)malloc(strlen(routeJSON) + strlen(otherDataJSON) + 50);
    sprintf(fullJSON, "{\"route\":%s,\"otherData\":%s}", routeJSON, otherDataJSON);
    
    char *ptr = (char *)realloc(string, strlen(string) + strlen(fullJSON) + 10);
    strcat(ptr, fullJSON);

    free(routeJSON);
    
    return ptr;
}

char *getTracksComponent(char *filename)
{
    GPXdoc *doc = createGPXdoc(filename);
    
    char *string = (char *)malloc(5);
    strcpy(string, "[");

    ListIterator li = createIterator(doc->tracks);
    void *elem = nextElement(&li);
    while(elem != NULL)
    {
        Track *tr = (Track *)elem;
        string = addTrackAndOD(string, tr);
        elem = nextElement(&li);
        if (elem != NULL){
            strcat(string, ",");
        }
    }
    strcat(string, "]");

    deleteGPXdoc(doc);
    return string;
}

char *addTrackAndOD(char *string, Track *tr)
{
    char *trackJSON = trackToJSON(tr);
    char *otherDataJSON = GPXDataListToJSON(tr->otherData);
    
    char *fullJSON = (char *)malloc(strlen(trackJSON) + strlen(otherDataJSON) + 50);
    sprintf(fullJSON, "{\"track\":%s,\"otherData\":%s}", trackJSON, otherDataJSON);
    
    char *ptr = (char *)realloc(string, strlen(string) + strlen(fullJSON) + 10);
    strcat(ptr, fullJSON);

    free(trackJSON);
    
    return ptr;
}

int getNumTrackPts(const Track *tr){
    int n = 0;
    ListIterator li = createIterator(tr->segments);
    void *elem = nextElement(&li);
    while(elem != NULL){
        TrackSegment *seg = (TrackSegment *)elem;
        n += seg->waypoints->length;
        elem = nextElement(&li);
    }
    return n;
}

char *GPXDataToJSON(GPXData *otherData){

    char *JSON;
    if(otherData->name[0] < '0' || otherData->name[0] > 'z'){
        JSON = (char*)malloc(5);
        strcpy(JSON,"");
    }
    else if(otherData->value[0] < '0' || otherData->value[0] > 'z'){
        JSON = (char*)malloc(5);
        strcpy(JSON,"");
    } else {
        JSON =(char *)malloc(strlen(otherData->name) + strlen(otherData->name) + 100);
        char *line = strtok(otherData->value, "\n");
        sprintf(JSON, "{\"name\":\"%s\",\"value\":\"%s\"}", otherData->name, line);
    }
    return JSON;
}

char *GPXDataListToJSON(List *list){
    char *JSON = (char *)malloc(3);
    strcpy(JSON, "[");
    ListIterator li = createIterator(list);
    void *elem = nextElement(&li);
    while(elem != NULL){
        GPXData *od = (GPXData *)elem;
        char *string = GPXDataToJSON(od);
        JSON = (char *)realloc(JSON, strlen(JSON)+strlen(string) + 10);
        strcat(JSON, string);
        free(string);
        elem = nextElement(&li);
        if(elem != NULL){
            strcat(JSON, ",");
        }
    }
    
    strcat(JSON, "]");

    return JSON;
}

char *addGPXDataJSON(int *size, GPXData *dt, char *string, int *badJSON){
    char *otherDataString = GPXDataToJSON(dt);
    if (strcmp(otherDataString, "") == 0){
        (*badJSON)++;
        return string;
    }
    (*size) += strlen(otherDataString) + 50;
    char *JSON = (char *)realloc(string, (*size));
    strcat(JSON, otherDataString);
    free(otherDataString);
    return JSON;
}

void renameComponent(char *filename, char *type, char *idx, char *newname)
{
    GPXdoc *doc = createGPXdoc(filename);

    int j = atoi(idx) - 1;
    
    if(strcmp(type, "Route") == 0){
        ListIterator li = createIterator(doc->routes);
        void *elem = nextElement(&li);
        for(int i = 0; (i<j && elem!=NULL); i++)
        {
            elem = nextElement(&li);
        }
        Route *rt = (Route *)elem;
        rt->name = realloc(rt->name, strlen(newname)+10);
        strcpy(rt->name, newname);
    }

    if(strcmp(type, "Track") == 0){
        ListIterator li = createIterator(doc->tracks);
        void *elem = nextElement(&li);
        for(int i = 0; (i<j && elem!=NULL); i++)
        {
            elem = nextElement(&li);
        }
        Track *tr = (Track *)elem;
        tr->name = (char *)realloc(tr->name, strlen(newname) + 10);
        strcpy(tr->name, newname);
    }
    writeGPXdoc(doc, filename);
    deleteGPXdoc(doc);
}

char *validateFile(char *filename, char *path){
    char *isValid = (char *)malloc(7);
    char *schemaPath = (char *)malloc(strlen(path) + 30);
    strcpy(schemaPath, path);
    strcat(schemaPath, "/parser/resources/gpx.xsd");
    GPXdoc *doc = createValidGPXdoc(filename, schemaPath);
    strcpy(isValid, (doc != NULL  ? "true":"false"));
    free(schemaPath);
    return isValid;
}

void addRouteFromApp(char *addRouteFile, char *filename, char *JSONstring){

    char name[MAX_BUF];
    char longitudes[MAX_BUF];
    char latitudes[MAX_BUF];

    sscanf(JSONstring, "{\"name\":\"%[0-9a-zA-Z_ ]\",\"longitudes\":[%[0-9,.-]],\"latitudes\":[%[0-9,.-]]}",name, longitudes, latitudes);

    GPXdoc *doc = createGPXdoc(addRouteFile);

    Route *rte = malloc(sizeof(Route));
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rte->name = NULL;

    printf("Name: %s\n", name);
    rte->name = (char *)malloc(strlen(name)+10);
    strcpy(rte->name, name);

    printf("Lon: %s\n", longitudes);
    printf("Lat: %s\n", latitudes);

    char *val = strtok(longitudes, ",");
    while(val != NULL){
        Waypoint *pt = (Waypoint *)malloc(sizeof(Waypoint));
        pt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
        pt->longitude = atof(val);
        pt->name = malloc(5);
        strcpy(pt->name, "");
        addWaypoint(rte, pt);
        val = strtok(NULL, ",");
    }

    val = strtok(latitudes, ",");
    ListIterator li = createIterator(rte->waypoints);
    void *elem = nextElement(&li);
    while(elem != NULL){
        Waypoint *pt = (Waypoint *)elem;
        pt->latitude = atof(val);
        elem = nextElement(&li);
        if(elem != NULL){
            val = strtok(NULL, ",");
        }
    }

    addRoute(doc, rte);
    
    writeGPXdoc(doc, filename);

    deleteGPXdoc(doc);
}

void createGPXFromApp(char *filename, char *GPXJSON){
    GPXdoc *doc = JSONtoGPX(GPXJSON);
    writeGPXdoc(doc, filename);
}

char *componentsBetween(char *filename, char *lon1, char *lat1, char *lon2, char *lat2, char *delta){
    GPXdoc *doc = createGPXdoc(filename);
    List *routes = getRoutesBetween(doc, atof(lat1), atof(lon1), atof(lat2), atof(lon2), atof(delta));
    List *tracks = getTracksBetween(doc, atof(lat1), atof(lon1), atof(lat2), atof(lon2), atof(delta));
    
    char *output = (char *)malloc(5);

    ListIterator li = createIterator(routes);
    void *elem = nextElement(&li);
    while(elem != NULL){
        // Route *rt = (Route *)elem;
        // output = addRouteToListBetween();
        elem = nextElement(&li);
    }

    li = createIterator(tracks);
    elem = nextElement(&li);
    while(elem != NULL){
        // Track *rt = (Track *)elem;
        elem = nextElement(&li);
    }
    strcat(output, "}");
    deleteGPXdoc(doc);
    return output;
}

char *getRoutesBetweenJSON(char *path, char *filename, char *lon1, char *lat1, char *lon2, char *lat2, char *delta)
{
    GPXdoc *doc = createGPXdoc(path);
    List *routes = getRoutesBetween(doc, atof(lat1), atof(lon1), atof(lat2), atof(lon2), atof(delta));
    char *routesListJSON = routeListToJSON(routes);
    char *out = malloc(strlen(filename) + strlen(routesListJSON) + 50);
    sprintf(out, "{\"name\":\"%s\",\"routes\":%s}",filename, routesListJSON);

    return out;
}

char *getTracksBetweenJSON(char *path, char *filename, char *lon1, char *lat1, char *lon2, char *lat2, char *delta)
{
    GPXdoc *doc = createGPXdoc(path);
    List *tracks = getTracksBetween(doc, atof(lat1), atof(lon1), atof(lat2), atof(lon2), atof(delta));
    char *tracksListJSON = trackListToJSON(tracks);
    char *out = malloc(strlen(filename) + strlen(tracksListJSON) + 50);
    sprintf(out, "{\"name\":\"%s\",\"tracks\":%s}",filename, tracksListJSON);
    return out;
}

char *getPoints(char *path, char *id){
    
    int i = atoi(id);
    GPXdoc *doc = createGPXdoc(path);

    char *JSON = malloc(3);
    strcpy(JSON, "[");
    
    ListIterator li = createIterator(doc->routes);
    void *elem = nextElement(&li);
    for(int j=0; (j<i && elem!=NULL);j++){
        elem = nextElement(&li);
    }
    Route *rt = (Route *)elem;

    int idx = 0;
    li = createIterator(rt->waypoints);
    elem = nextElement(&li);
    while(elem!=NULL){
        Waypoint *pt = (Waypoint *)elem;
        JSON = addWaypointJSON(pt, JSON, idx++);
        elem = nextElement(&li);
        if(elem != NULL){
            strcat(JSON, ",");
        }
    }
    strcat(JSON, "]");

    deleteGPXdoc(doc);
    return JSON;
}

char *addWaypointJSON(Waypoint *pt, char *JSON, int index){
    char *ptJSON = malloc(200);

    if(strcmp(pt->name,"") != 0){
        sprintf(ptJSON, "{\"idx\":%d,\"latitude\":%lf,\"longitude\":%lf,\"name\":\"%s\"}", index, pt->latitude, pt->longitude, pt->name);
    } else {
        sprintf(ptJSON, "{\"idx\":%d,\"latitude\":%lf,\"longitude\":%lf,\"name\":\"%s\"}", index, pt->latitude, pt->longitude, "NULL");
    }

    char *ptr = (char *)realloc(JSON, strlen(JSON)+strlen(ptJSON)+20);
    strcat(ptr, ptJSON);
    free(ptJSON);
    return ptr;
}