#include "creds.h"
#include "oci.h"

/* enter ssid and pass */
const char ssid[] = "";
const char pass[] = "";

/* update OS host with the proper endpoint for your region */
char osHost[] = "objectstorage.us-phoenix-1.oraclecloud.com";

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

  // Serial.println("\n*** list Object Storage buckets ***\n");
  listBuckets();

}

void loop() {
}

void listBuckets() {
  /* populate namespace and compartment OCID */
  char* osPath = "/n/[namespace]/b/?compartmentId=[compartment ocid]";

  /* 
    create a request to retrieve OS buckets
    to ensure a secure connection, pass the Root CA Cert 
    as the 6th argument
  */
  OciApiRequest listBucketsRequest(osHost, osPath, oci.HTTP_METHOD_GET, {}, 0, NULL);

  /* 
    create an object to store the result
    pass the array of headers to retrieve from the response
  */
  OciApiResponse listBucketsResponse;
  
  oci.apiCall(listBucketsRequest, listBucketsResponse);

  if( listBucketsResponse.statusCode == 200 ) {
    Serial.println("List Buckets Response:");
  }
  else {
    Serial.println(listBucketsResponse.errorMsg);
  }
}