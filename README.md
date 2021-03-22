# OCI APIs (Unoffical Library)

The purpose of this library is to let you invoke Oracle Cloud Infrastructure (OCI) REST APIs directly from your device. 

## Before You Use This...
You should be familiar with the OCI REST APIs:

* [Using REST APIs](https://docs.oracle.com/en-us/iaas/Content/API/Concepts/usingapi.htm)

* [REST API Endpoints](https://docs.oracle.com/en-us/iaas/api/#/)

You should have already created the required keys and collected the necessary OCIDs.  See [Required Keys and OCIDs](https://docs.oracle.com/en-us/iaas/Content/API/Concepts/apisigningkey.htm#Required_Keys_and_OCIDs).

## About

This library exists to make it easier to make calls to the OCI APIs from your microcontroller. The [request signature](https://docs.oracle.com/en-us/iaas/Content/API/Concepts/signingrequests.htm) process is somewhat complex, and it can be tricky to sign an HTTPS request from a memory constrained device. I created this library to simplify the process.

**Please note!** This library has only been successfully tested on an ESP-32. It has not been tested on any other board, and probably won't work on any other board due to memory constraints and dependent libraries.

## Documentation

The latest documentation always lives at: https://recursivecodes.github.io/oci-rest-api-for-esp-32/html/index.html

## Usage

### Include the Library

```c_cpp
#include "oci.h"
```

### Declare Variables for Keys and OCIDs

Declare some variables to hold your keys and OCIDs. Keep this **out of source control**!

```c_cpp
char tenancyOcid[] = "ocid1.tenancy.oc1..[redacted]]";
char userOcid[] = "ocid1.user.oc1..[redacted]";
char keyFingerprint[] = "1z:[redacted]:99";
char* apiKey = \
"-----BEGIN RSA PRIVATE KEY-----\n"\
"MI[redacted]4h\n"\
...
"Q0[redacted]PGj\n"\
"-----END RSA PRIVATE KEY-----\n";
```

### Construct OCI Profile

Pass in your `tenancyOcid`, `userOcid`, `keyFingerprint`, `apiKey`. If your private key is password protected, pass the password in as the 5th argument.

```c_cpp
OciProfile ociProfile(tenancyOcid, userOcid, keyFingerprint, apiKey);
```

### Construct OCI Instance

Construct an instance of the `Oci` class, passing in your profile. This will initialize the API class and configure the NTP server needed to obtain a timestamp to include with each request.

```c_cpp
Oci oci(ociProfile);
```

### Request Object

Construct a request object. Pass in the REST endpoint host, the path, HTTP method as the first 3 arguments. 

```c_cpp
OciApiRequest listBucketsRequest(osHost, osPath, oci.HTTP_METHOD_GET, {}, 0,  objectStorageRootCert);
```

The example above makes a secure request because a copy of the endpoint's Root CA Cert is passed in as the final argument. You can get a copy of the Root CA Cert many ways. Here's an example using `openssl` on *nix compatible systems:

```bash
$ openssl s_client -connect objectstorage.us-phoenix-1.oraclecloud.com:443 -showcerts
```

If you want to make the request insecure (the Root CA Cert will not be validated), pass `NULL` in instead of the cert.

```c_cpp
OciApiRequest listBucketsRequest(osHost, osPath, oci.HTTP_METHOD_GET, {}, 0,  NULL);
```

If you want/need to add request headers to the API call, construct and pass an array of `Header` structs in argument 4 and the length of that array in argument 5.

```c_cpp
/* headers to add to the request */
Header reqHeaders[] = { {"opc-client-request-id", "1234-ABCD"} };
OciApiRequest listBucketsRequest(osHost, osPath, oci.HTTP_METHOD_GET, reqHeaders, 1, objectStorageRootCert);
```

### Response Object

Construct a response object. This will ultimately hold the results (or any errors) of your API call. 

```c_cpp
OciApiResponse listBucketsResponse;
```

If you want/need to retrieve any headers from the API response, construct and add an array of `Header` structs. Just add the name of the header to retrieve, the value will be populated when the API call is complete.

```c_cpp
/* headers to retrieve from the result (name only) */
Header resHeaders[] = { {"opc-request-id"} };
OciApiResponse listBucketsResponse(resHeaders, 1);
```


### Call API

Call the `apiCall()` method of `Oci`, passing the request and response objects.

```c_cpp
oci.apiCall(listBucketsRequest, listBucketsResponse);
```

If successful, the `statusCode` property of the response object will be populated with `200`. You can then print/handle the result as needed. In this example, I'm deserializing the JSON string into an object.

```c_cpp
if( listBucketsResponse.statusCode == 200 ) {
    // print the `opc-request-id` from the response headers
    Serial.println(resHeaders[0].headerValue);
    // deserialize and pretty print the response
    Serial.println("List Buckets Response:");
    DynamicJsonDocument doc(6000);
    deserializeJson(doc, listBucketsResponse.response);
    serializeJsonPretty(doc, Serial);  
  }
  else {
    Serial.println(listBucketsResponse.errorMsg);
  }
```

The previous example might produce output such as this:

```json
[
  {
    "namespace": "toddrsharp",
    "name": "archive-demo",
    "compartmentId": "ocid1.compartment.oc1...",
    "createdBy": "ocid1.saml2idp.oc1.../...",
    "timeCreated": "2020-06-18T17:49:14.490Z",
    "etag": "11e0fffe-280c-4311-8d72-755805766815",
    "freeformTags": null,
    "definedTags": null
  },
  {
    "namespace": "toddrsharp",
    "name": "custom-images",
    "compartmentId": "ocid1.compartment.oc1...",
    "createdBy": "ocid1.saml2idp.oc1.../...",
    "timeCreated": "2019-10-24T17:52:47.425Z",
    "etag": "00c17467-2ac3-4257-aef0-a619aa4cab2b",
    "freeformTags": null,
    "definedTags": null
  }
]
```