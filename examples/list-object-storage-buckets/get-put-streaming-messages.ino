#include "creds.h"
#include "oci.h"

/* enter ssid and pass */
const char ssid[] = "";
const char pass[] = "";

/* update streaming host with the proper endpoint for your region */
char streamingHost[] = "streaming.us-phoenix-1.oci.oraclecloud.com";
/* populate your stream OCID */
char demoStreamOcid[] = "";

/* enter OCIDs and key fingerprint and key */
char tenancyOcid[] = "";
char userOcid[] = "";
char keyFingerprint[] = "";
char* apiKey = \
"-----BEGIN RSA PRIVATE KEY-----\n"\
"-----END RSA PRIVATE KEY-----\n";

OciProfile ociProfile(tenancyOcid, userOcid, keyFingerprint, apiKey);
Oci oci(ociProfile);

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.println(F("."));
  }
  Serial.print("Connected.");

  Serial.println("\n*** send message to stream ***\n");
  putMessage();

  Serial.println("\n*** get cursor ***\n");
  getCursor();

  Serial.println("\n*** get messages ***\n");
  getMessages();

}

void loop() {
}


void postMessage() {
  
  /* base64 encode the message value */
  const char *input = "{\"msg\": \"hello, world!\"}";
  unsigned char output[64]; 
  size_t msgOutLen;
  mbedtls_base64_encode(output, 64, &msgOutLen, (const unsigned char*) input, strlen((char*) ((const unsigned char*) input)));

  /* construct a JSON object to contain the message to POST */
  char message[150] = "{ \"messages\": [ { \"key\": null, \"value\": \"";
  strcat(message, (char*) output);
  strcat(message, "\" } ] }");

  // the path to the API endpoint containing the stream OCID
  char postMsgPath[120] = "/20180418/streams/";
  strcat(postMsgPath, demoStreamOcid);
  strcat(postMsgPath, "/messages");
  
  OciApiRequest postMessageRequest(streamingHost, postMsgPath, oci.HTTP_METHOD_POST, {}, 0, NULL, message);
  OciApiResponse postMessageResponse;
  oci.apiCall(postMessageRequest, postMessageResponse);

  if( postMessageResponse.statusCode == 200 ) {
    Serial.println("Post Message Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, postMessageResponse.response);
    serializeJsonPretty(doc, Serial);  
  }
  else {
    Serial.println(postMessageResponse.errorMsg);
  }  
}

char* cursor;

void getCursor() {
  char cursorPath[130] = "/20180418/streams/";
  strcat(cursorPath, demoStreamOcid);
  strcat(cursorPath, "/cursors");

  char createCursorBody[] = "{\"partition\": \"0\", \"type\": \"LATEST\"}";
  OciApiRequest getCursorRequest(streamingHost, cursorPath, oci.HTTP_METHOD_POST, {}, 0, streamingServiceRootCert, createCursorBody);
  OciApiResponse getCursorResponse;
  oci.apiCall(getCursorRequest, getCursorResponse);

  if( getCursorResponse.statusCode == 200 ) {
    Serial.println("Get Cursor Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, getCursorResponse.response);
    serializeJsonPretty(doc, Serial);
    int cursorLen = strlen(doc["value"])+1;
    cursor = (char*)malloc(cursorLen);
    strncpy(cursor, (const char*) doc["value"], cursorLen);
  }
  else {
    Serial.println(getCursorResponse.errorMsg);
  }
}

void getMessages() {
  char getMsgPath[600] = "/20180418/streams/";
  strcat(getMsgPath, demoStreamOcid);
  strcat(getMsgPath, "/messages?cursor=");
  strcat(getMsgPath, cursor);
  strcat(getMsgPath, "&limit=2");
  
  Header getMsgsHeaders[] = { {"opc-next-cursor"} };
  OciApiRequest getMsgsRequest(streamingHost, getMsgPath, oci.HTTP_METHOD_GET, getMsgsHeaders, 1);
  OciApiResponse getMsgsResponse;
  oci.apiCall(getMsgsRequest, getMsgsResponse);
  
  if( getMsgsResponse.statusCode == 200 ) {
    Serial.println("Get Messages Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, getMsgsResponse.response);
    serializeJsonPretty(doc, Serial);
    int newCursorLen = strlen(getMsgsHeaders[0].headerValue)+1;
    cursor = (char*)malloc(newCursorLen);
    strncpy(cursor, (const char*) getMsgsHeaders[0].headerValue, newCursorLen);
  }
  else {
    Serial.println(getMsgsResponse.errorMsg);
  } 
}