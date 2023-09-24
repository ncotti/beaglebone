# HTTP 1.1 Headers

For a full document on HTTP, check the [MDN HTTP Guide](https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP).

In this file, I will dissect an HTTP client request, and format an HTTP server response. Base upon [THIS page](https://www.geeksforgeeks.org/http-headers/) and [THIS page](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers).

## Basic HTTP background

HTTP, in the OSI model, is way up there, in the application layer. This means that all the internet connection has been handled already.

![OSI model](Images/Pasted%20image%2020230920134727.png)

An internet connection is ALWAYS started from the client, normally a web browser. They will request an element from the server, and the server will provide.

The HTTP protocol is **stateless**, this means that previous request don't have any correlation with the current request (unless *cookies* are used). Therefore, by default, each new HTTP request is made on a new socket, although the socket can be kept open using the `Connection: keep-alive` header.
## Client request

A generic HTTP/1.1 request will look something like this: first, the **GET** method, followed by the route from the server, the HTTP protocol version, and then a list of headers like `Header: <value>`.

```http
GET <route> HTTP/1.1
Host: <host_name>:<port>
Connection: <keep-alive | close>
Accept: <MIME_type>/<MIME_subtype>
Accept-Language: <language>
Accept-Encoding: <encoding>
User-Agent: <os_web_browser>
Referer: <Origin_URL>
```

An example request taken from the web browser:

```http
GET /guide-step/4150/http-referer-explanation HTTP/1.1
Host: 78.14.10.123:443
Connection: keep-alive
Referer: https://www.google.com/
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate, br
Accept-Language: es-419,es;q=0.9,en;q=0.8
Sec-Ch-Ua: "Not/A)Brand";v="99", "Google Chrome";v="115", "Chromium";v="115"
Sec-Ch-Ua-Mobile: ?0
Sec-Ch-Ua-Platform: "Linux"
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: cross-site
Sec-Fetch-User: ?1
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36
```

Up next there is a revision of the most common headers and values. All headers should be treated as *case insensitive*.

### HTTP headers with client information

The **User-Agent** is a request header that allows network protocol peers to identify the Operating System and Browser of the web-server.

```http
User-Agent: <product>/<product-version> <comment>

User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36
```

#### Sec-CH-UA:  User-Agent Client Hints

Instead of using the **User-Agent:** header, the **User-Agent Client Hints** provide the same information, but avoid the need for parsing. The values "?0" and "?1" correspond to a boolean "false" and "true".

```http
Sec-CH-UA-Mobile: <?0 | ?1>
Sec-CH-UA-Platform: <OS>
Sec-CH-UA: <Web_browser>
```

#### Sec-Fetch 

This headers have information about the origin of the request:
* Site: From which website.
* Mode: How is being requested (see [CORS](https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS)).
* Dest: From which HTML element was requested.
* User: If an user made the request (always "?1").

```http
Sec-Fetch-Site: <cross-site | same-origin | same-site | none>
Sec-Fetch-Mode: <cors | navigate | no-cors | same-origin | websocket>
Sec-Fetch-Dest: <audio | image | script | video | style>
Sec-Fetch-User: ?1
```

### HTTP headers with content negotiation

The **Accept header** is used to inform the server by the client that which content type is understandable by the client expressed as MIME-types.

```http
Accept: <MIME_type>/<MIME_subtype> | <MIME_type>/* | */*
Accept: text/html, text/plain, image/*
```

The **HTTP Accept-Language header** tells the server about all the languages that the client can understand. If `q=` is used, a preference level between "0" and "1" is set for that encoding.

```http
Accept-Language: <language>;q=
Accept-Language: es-419,es;q=0.9,en;q=0.8
```

The **HTTP headers Accepts-Encoding** is usually a comparison algorithm of request header. All the HTTP client used to tell the server which encoding or encoding it supports. Then the server will respond in any of the supporting encoding formats. The server selects any one of the proposals, uses it and informs the client of its choice with the **Content-Encoding** response header.

```http
Accept-encoding: gzip, identity, compress, deflate, br, *
```

### Connection headers

The **HTTP Connection header** allows the sender or client to specify options that are desired for that particular connection. Instead of opening a new connection for every single request/response, Connection helps in sending or receiving multiple HTTP requests/responses using a single TCP connection. It also controls whether or not the network stays open or close after the current transaction finishes.

```http
Connection: keep-alive | close
```

The **HTTP Host** represents the **domain name** of the server. It may also represent the **Transmission Control Protocol (TCP)** port number which the server uses.

```http
Host: <host>:<port>

Host: www.example.com:80
```

**Referer** contains the direction of the previous web page. For example, if you click on an add from "www.page1.com", to get to the page, that will be the value stored. If you searched it from google, you will get "www.google.com" instead. It can be used by the server to identify from where the visitors are coming.

```http
Referer: <url>
```

## Server response

A generic server response looks like this: First, put the HTTP version, and one of the following [HTTP state codes](https://developer.mozilla.org/es/docs/Web/HTTP/Status). Then, follow with some headers, a purposefully blank line (double "\\n"), and then the content of the response. 

```http
HTTP/1.1 <HTTP_state_code> <HTTP_state_string>
Date: day-name, day month year hour:minute:second GMT
Server: <product>
Content-Length: <byte_size>
Content-Encoding: <compression_algorithms>
Content-Language: <language>
Content-Type: <MIME_type>/<MIME_subtype>; charset=<encoding>

<Response>
```

Meanwhile, the response got from the server to the client request in the previous section is:

```http
HTTP/1.1 200 OK
Date: Wed, 20 Sep 2023 13:35:50 GMT
Expires: Sun, 19 Nov 1978 05:00:00 GMT
Server: nginx
Content-Length: 1957
Content-Encoding: gzip
Content-Language: es
Content-Type: text/html; charset=UTF-8

<html></html>
```

### HTTP headers with server information

**Date HTTP header** contains the date and time at which the message was generated.

```http
Date: day-name, day month year hour:minute:second GMT
Date: Wed, 16 Oct 2019 07:28:00 GMT
```

The **`Server`** header describes the software used by the origin server that handled the request.

```http
Server: <product>
Server: Apache/2.4.1 (Unix)
```

The last modified response header is a header sent by the server specifying the date of the last modification of the requested source. It's used by the caching mechanism.

```http
Last-Modified: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
Last-Modified: Tue, 15 Oct 2019 12:45:26 GMT
```

The **connection** header should be specified aswell, letting the client know if the server will keep sending data from the same TCP socket.

```http
Connection: keep-alive | close
```

### Message body information, Content

The request had a bunch of "Accept" statements. Now, we should respond to that.

The **`Content-Length`** header indicates the size of the message body, in bytes, sent to the recipient.

```http
Content-Length: <length>
Content-Length: 1957
```

The **`Content-Type`** representation header is used to indicate the media type of the resource.

```http
Content-Type: <MIME_type>/<MIME_subtype>; charset=<encoding>
Content-Type: text/html; charset=utf-8
```

The **`Content-Encoding`** lists any encodings that have been applied to the representation (message payload), and in what order. If no compression was used, set this header to "identity".

```http
Content-Encoding: gzip     // Compressed with gzip
Content-Encoding: identity // Uncompressed
Content-Encoding: deflate, gzip // Multiple, in the order in which they were applied
```

The **`Content-Language`** is used to **describe the language(s) intended for the audience**, so users can differentiate it according to their own preferred language.

```http
Content-Language: en-US
```

