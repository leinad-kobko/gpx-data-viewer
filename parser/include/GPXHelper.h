#include <stdio.h>

#define MAX_BUF 50
#define EARTHR 6371000

/* GPXParser.c */
void populateLists(List *waypoints, List *routes, List *tracks, xmlNode *root_element);

/* PathMaker.c */
GPXData *createGPXData(xmlNode *node);
Waypoint *createWaypoint(xmlNode *node);
Route *createRoute(xmlNode *node);
TrackSegment *createTrackSegment(xmlNode *node);
Track *createTrack(xmlNode *node);

void createXmlOtherNode(GPXData *data, xmlNodePtr parent, xmlNsPtr ns);
void createXmlPtNode(Waypoint *wpt, xmlNodePtr parent, xmlNsPtr ns, char *wptType);
void createXmlRteNode(Route *rte, xmlNodePtr parent, xmlNsPtr ns);
void createXmlSegNode(TrackSegment *seg, xmlNodePtr parent, xmlNsPtr ns);
void createXmlTrkNode(Track *trk, xmlNodePtr parent, xmlNsPtr ns);

xmlDocPtr GPXToXML(GPXdoc *gpxDoc);

float distanceBetweenPoints(Waypoint *pt1, Waypoint *pt2);
void dummyDelete(void *data);

char *addRouteJSON(Route *rt, char *string, int *size);
char *addTrackJSON(Track *tr, char *string, int *size);

void test(char *f1, char *f2);

char *fileToJSON(char *filename);
char *getRoutesComponent(char *filename);
char *getTracksComponent(char *filename);
int getNumTrackPts(const Track *tr);
char *GPXDataToJSON(GPXData *otherData);
char *GPXDataListToJSON(List *);
char *addGPXDataJSON(int *size, GPXData *dt, char *string, int *badJSON);
void renameComponent(char *filename, char *type, char *id, char *newname);
char *validateFile(char *filename, char *path);
void addRouteFromApp(char *addRouteFile, char *filename, char *JSONstring);
void createGPXFromApp(char *filename, char *GPXJSON);
char *componentsBetween(char *filename, char *lon1, char *lat1, char *lon2, char *lat2, char *delta);
char *addRouteAndOD(char *string, Route *rt);
char *addTrackAndOD(char *string, Track *tr);
char *getRoutesBetweenJSON(char *path, char *filename, char *lon1, char *lat1, char *lon2, char *lat2, char *delta);
char *getTracksBetweenJSON(char *path, char *filename, char *lon1, char *lat1, char *lon2, char *lat2, char *delta);
char *getPoints(char *path, char *id);
char *addWaypointJSON(Waypoint *pt, char *JSON, int index);