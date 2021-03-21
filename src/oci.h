#ifndef Oci_h
#define Oci_h

#include "Arduino.h"
#include <HTTPClient.h>
#include "WiFiClientSecure.h"
#include "time.h"
#include "mbedtls/timing.h"
#include "mbedtls/sha256.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/rsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"

/** \struct header 
 * A Header object is a simple key/value pair. 
 * It is used by both  OciApiRequest and OciApiResponse.
 */
struct header {
  char* headerName; //!< The header name
  char* headerValue; //!< The header value
  /**
   * Construct a Header with just a name
   * @param hN The header name
   */
  header(char* hN) {
    this->headerName = hN;
  };
  /** 
   * Construct a Header with a name and value
   * @param hN The header name
   * @param hV The header value
   */
  header(char* hN, char* hV) {
    this->headerName = hN;
    this->headerValue = hV;
  };
};
/*! Creates a type name for Header */
typedef struct header Header;


/** \struct ociApiRequest 
 * A request contains all of the information 
 * required to make a call to the OCI API.
 */
struct ociApiRequest {
  char* host; //!< The REST API endpoint host. Ex: objectstorage.us-phoenix-1.oraclecloud.com
  char* path; //!< The path. Ex: "/n/"
  const char* requestMethod; //!< The HTTP method. See Oci for `const` values.
  char* endpointCert=NULL; //!< The text value of the Root CA cert for the API endpoint. Used to make secure connections. For examp, run `openssl s_client -connect objectstorage.us-phoenix-1.oraclecloud.com:443 -showcerts` and find the value of the Root CA Cert.
  char* content = NULL; //!< The body to be passed to the API call.
  char* contentType="application/json"; //!< The content type. 
  Header* requestHeaders; //!< An array of Header objects (set both key and value)
  int requestHeaderCount; //!< The count of headers in the requestHeaders array 
  
  /** An OCI API request.
   * @param host The REST API endpoint host. Ex: objectstorage.us-phoenix-1.oraclecloud.com
   * @param path The path. Ex: "/n/"
   * @param requestMethod The HTTP method. See Oci for `const` values.
   * @param requestHeaders An array of Header objects (set both key and value)
   * @param requestHeaderCount The count of headers in the requestHeaders array 
   * @param endpointCert The text value of the Root CA cert for the API endpoint. Used to make secure connections. For examp, run `openssl s_client -connect objectstorage.us-phoenix-1.oraclecloud.com:443 -showcerts` and find the value of the Root CA Cert.
   * @param content The body to be passed to the API call.
   * @param contentType The content type
   */
  ociApiRequest(
    char* host, 
    char* path, 
    const char* requestMethod, 
    Header* requestHeaders={}, 
    int requestHeaderCount=0, 
    char* endpointCert=NULL, 
    char* content=NULL, 
    char* contentType="application/json" 
  ){
    this->host = host;
    this->path = path;
    this->requestMethod = requestMethod;
    this->endpointCert = endpointCert;
    this->content = content;
    this->contentType = contentType;
    this->requestHeaders = requestHeaders;
    this->requestHeaderCount = requestHeaderCount;
  }
};
typedef struct ociApiRequest OciApiRequest;

/** \struct ociApiResponse
 * Contains the response from an API call.
 * Pass an array of Header objects if you want to retrieve headers sent back from the API. 
 */
struct ociApiResponse {
  String response; //!< The string returned from the API call. Usually JSON.
  int statusCode; //!< The HTTP status code returned
  String opcRequestId; //!< The `opc-request-id`
  String errorMsg; //!< Any error message returned from the API call
  Header* responseHeaders; //!< An array of headers to collect from the API call
  int responseHeaderCount; //!< The count of headers to collect
   
  /** Construct a response with an array of headers and the count
   * @param responseHeaders An array of headers (name only)
   * @param responseHeaderCount The count of headers in the array
   */
  ociApiResponse(Header* responseHeaders={},int responseHeaderCount=0){
    this->responseHeaders = responseHeaders;
    this->responseHeaderCount = responseHeaderCount;
  };
  
  /** Construct a full response object. Used internally.
   * @param response The response text
   * @param statusCode The HTTP status code. Ex. 200.
   * @param opcRequestId The `opc-request-id`.
   * @param errorMsg The error returned from the API
   * @param responseHeaders An array of headers (name only)
   * @param responseHeaderCount The count of headers in the array 
   */
  ociApiResponse(String response, int statusCode, String opcRequestId, String errorMsg, Header* responseHeaders={},int responseHeaderCount=0){
    this->response = response;
    this->statusCode = statusCode;
    this->opcRequestId = opcRequestId;
    this->errorMsg = errorMsg;
    this->responseHeaders = responseHeaders;
    this->responseHeaderCount = responseHeaderCount;
  }
};
typedef struct ociApiResponse OciApiResponse;

/** \struct ociProfile
 * Contains the OCIDs and key & key related data necessary to sign the request.
 * 
 */
struct ociProfile {
  char* tenancyOcid; //!< The tenancy OCID
  char* userOcid; //!< The user OCID
  char* keyFingerprint; //!< The key fingerprint
  char* privateKey; //!< The private key text. must be null terminated! (have a newline at the end of the text)
  char* privateKeyPassphrase; //!< Optional - the private key password (for protected keys)

  /** Construct an instance of OciProfile. 
   */
  ociProfile(){};
  
  /** Construct an instance of OciProfile. 
   * @param tenancyOcid The tenancy OCID
   * @param userOcid The user OCID
   * @param keyFingerprint The key fingerprint
   * @param privateKey The private key text. must be null terminated! (have a newline at the end of the text)
   * @param privateKeyPassphrase Optional - the private key password (for protected keys)
   */
  ociProfile(char* tenancyOcid, char* userOcid, char* keyFingerprint, char* privateKey, char* privateKeyPassphrase=NULL) {
    this->tenancyOcid = tenancyOcid;
    this->userOcid = userOcid;
    this->keyFingerprint = keyFingerprint;  
    this->privateKey = privateKey;
    this->privateKeyPassphrase = privateKeyPassphrase;
  }
};
typedef struct ociProfile OciProfile;

/** The OCI API. 
 * 
 */
class Oci {
  public:
  
    const char* HTTP_METHOD_GET = "get"; //!< HTTP GET
    const char* HTTP_METHOD_POST = "post"; //!< HTTP POST
    const char* HTTP_METHOD_PUT = "put"; //!< HTTP PUT
    const char* HTTP_METHOD_PATCH = "patch"; //!< HTTP PATCH
    const char* HTTP_METHOD_DELETE = "delete"; //!< HTTP DELETE

    OciProfile ociProfile; //!< The OciProfile to use
    char* ntpServer = "pool.ntp.org"; //!< The ntpServer
    long gmtOffset = 0; //!< The offset from GMT to use for retrieving time. You shouldn't change this - requests should be signed with time at GMT.
    int daylightOffset = 0;  //!< The daylight savings offset. Should not change this either.
    
    /** Construct a new instance of the OCI API. 
     * @param profile A configured OciProfile to be used to sign API calls
     * @param timeServer The NTP Server to be used to obtain the current time. Normally it should be left as default.
     * @param gmtOffsetSeconds The offset in seconds from GMT for the time object. Leave this as 0, API calls must be signed with the current time in GMT
     * @param daylightOffsetSeconds Offset in seconds for DST. Leave as zero.
     */
    Oci(
        OciProfile profile, 
        char* timeServer="pool.ntp.org", 
        long gmtOffsetSeconds=0, 
        int daylightOffsetSeconds=0
      ){
      ociProfile = profile;
      ntpServer = timeServer;
      gmtOffset = gmtOffsetSeconds;
      daylightOffset = daylightOffsetSeconds;
      configTime((const long) gmtOffset, (const int) daylightOffset, (const char*) ntpServer);  
    }

    /** Used internally to hash, sign and base64 encode the auth header to sign the request. 
     * 
     */
    void encryptAndEncode(
        const unsigned char* toEncrypt/**< [in] toEncrypt the value to encrypt */, 
        unsigned char (&encoded)[1024]/**< [out] &encoded the hashed/signed/encoded output */
      ) {
      mbedtls_pk_context pk;
      mbedtls_pk_init(&pk);
      char* pwd = NULL;
      int pwdLen = 0;
      if(ociProfile.privateKeyPassphrase){
        pwd = ociProfile.privateKeyPassphrase;
        pwdLen = strlen(pwd);
      }
      mbedtls_pk_parse_key(&pk, (const unsigned char*) ociProfile.privateKey, strlen(ociProfile.privateKey)+1, (const unsigned char*) pwd, pwdLen);
      mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    
      int keyValid = mbedtls_rsa_check_privkey(rsa);
      unsigned char encryptBuffer[512];
      if( keyValid == 0 ) {
        byte hashResult[32];
        mbedtls_sha256(toEncrypt, strlen((char*) toEncrypt), hashResult, 0);
        int success = mbedtls_rsa_rsassa_pkcs1_v15_sign( rsa, NULL, NULL, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,  strlen((char*) hashResult), hashResult, encryptBuffer ); 
        size_t encodedOutLen;
        mbedtls_base64_encode( encoded, sizeof(encoded), &encodedOutLen, (const unsigned char*) encryptBuffer, sizeof(encryptBuffer) / 2 );
      }
    }
    
    /** Make a call to the OCI REST API.
     * 
     */
    void apiCall( 
        OciApiRequest request, /**< [in] The API request */
        OciApiResponse &response /** [out] The API response */
      ) {
      unsigned char encoded[1024];
      char timestamp[35];
      boolean putPost = request.requestMethod == HTTP_METHOD_POST || request.requestMethod == HTTP_METHOD_PUT;
      size_t contentLen = 0;
      if( request.content != NULL ) contentLen = strlen(request.content);
      unsigned char contentEncoded[64] = "";
      char contentLenStr[10] = "0";
      sprintf(contentLenStr, "%d", contentLen);
      
      {
        char signingString[1500] = "";
        strcat(signingString, "(request-target): ");
        strcat(signingString, request.requestMethod);
        strcat(signingString, " ");
        strcat(signingString, request.path);
        strcat(signingString, "\n");
        struct tm timeinfo;
        while(!getLocalTime(&timeinfo)){
          Serial.println("Failed to obtain time");
          delay(100);
        }
        strftime(timestamp, 35, "%a, %d %b %Y %H:%M:%S GMT" , &timeinfo);
        strcat(signingString, "date: ");
        strcat(signingString, timestamp);
        strcat(signingString, "\n");
        
        strcat(signingString, "host: ");
        strcat(signingString, request.host);
        if( putPost ) {
          strcat(signingString, "\n");
          {
            byte contentHash[32];
            mbedtls_sha256((const unsigned char*) request.content, strlen((char*) request.content), contentHash, 0);
            size_t contentEncodedOutLen;
            mbedtls_base64_encode(contentEncoded, 64, &contentEncodedOutLen, (const unsigned char*) contentHash, sizeof(contentHash) / 2);
            strcat(signingString, "x-content-sha256: ");
            strcat(signingString, (char*) contentEncoded);
            strcat(signingString, "\n");
          }
          // add content-length
          strcat(signingString, "content-length: ");
          strcat(signingString, contentLenStr);
          strcat(signingString, "\n");
    
          // add content-type
          strcat(signingString, "content-type: ");
          strcat(signingString, request.contentType);
        }
        encryptAndEncode((const unsigned char*) signingString, encoded);
      }
      
      char authHeader[1250] = "Signature version=\"1\",headers=\"(request-target) date host";
      if(putPost) strcat(authHeader, " x-content-sha256 content-length content-type");
      strcat(authHeader, "\",keyId=\"");
      strcat(authHeader, ociProfile.tenancyOcid);
      strcat(authHeader, "/");
      strcat(authHeader, ociProfile.userOcid);
      strcat(authHeader, "/");
      strcat(authHeader, ociProfile.keyFingerprint);
      strcat(authHeader, "\",algorithm=\"rsa-sha256\",signature=\"");
      strcat(authHeader, ((char*) encoded));
      strcat(authHeader, "\"");
    
      String url = "https://" + String(request.host) + String(request.path);
    
      /*
      Serial.println("Time:");
      Serial.println(timestamp);
      Serial.println("Auth Header:");
      Serial.println(String( (char*) authHeader));
      Serial.println("Content:");
      Serial.println(request.content);
      Serial.println("Content Length:");
      Serial.println(contentLenStr);
      Serial.println("Content Encoded:");
      Serial.println((char*) contentEncoded);
      */
      
      WiFiClientSecure *client = new WiFiClientSecure;
      if(request.endpointCert) {
        client->setCACert(request.endpointCert);
      }
      else {
        client->setInsecure();
      }
      {
        HTTPClient http;
        http.begin(*client, url);
        int statusCode = 0;
        http.addHeader("date", String(timestamp));
        http.addHeader("Authorization", String( (char*) authHeader) );
      
        if(putPost) {
          http.addHeader("x-content-sha256", String((char*) contentEncoded));
          // do not add content-length header, the httpclient adds it 
        }
        http.addHeader("content-type", String(request.contentType));
      
        // user defined headers - add them to the request
        for (int i=0; i<request.requestHeaderCount; i++) {
          char* hN = request.requestHeaders[i].headerName;
          char* hV = request.requestHeaders[i].headerValue;
          http.addHeader(hN, hV);
        }
        // headers to retrieve with response
        const char* keys[] = {};
        for (int i=0; i<response.responseHeaderCount; i++) {
          keys[i] = response.responseHeaders[i].headerName;
        }
        http.collectHeaders(keys, response.responseHeaderCount);
        if(request.requestMethod == HTTP_METHOD_GET) {
          statusCode = http.GET();
        }
        else if (request.requestMethod == HTTP_METHOD_POST) {
          statusCode = http.POST(request.content);
          
        }
        else if (request.requestMethod == HTTP_METHOD_PUT) {
          statusCode = http.PUT(String(request.content));
        }
        else if (request.requestMethod == HTTP_METHOD_DELETE) {
          statusCode = http.sendRequest("DELETE", request.content);
        }
        
        for (int i=0; i<response.responseHeaderCount; i++) {
          char* headerName = response.responseHeaders[i].headerName;
          String headerVal = http.header( (const char*) response.responseHeaders[i].headerName );
          int headerValLen = headerVal.length()+1;
          char headerValBuffer[headerValLen];
          headerVal.toCharArray(headerValBuffer, headerValLen);
          response.responseHeaders[i].headerValue = (char *)malloc(headerValLen);
          strncpy(response.responseHeaders[i].headerValue, (const char*) headerValBuffer, headerValLen);
        }
        
        if (statusCode > 0) { 
          response.response = http.getString();
          response.statusCode = statusCode;
        }
        else {
          response.errorMsg = http.errorToString(statusCode).c_str();
        }
        http.end();     
      }
    }
};
#endif
