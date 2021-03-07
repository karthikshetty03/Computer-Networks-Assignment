We often access the web through a proxy. Squid is a caching and forwarding HTTP web proxy. This is a program written using sockets in C language, without using any high level
libraries, to download the main page and logo through a squid proxy.
The proxy configuration is as follows:

### Server: 182.75.45.22
### Port: 13128
### Username: csf303
### Password: csf303

If we receive a HTTP 30x response, that is handled by following the redirection. 

    $ gcc http_proxy_download.c -o http_proxy_download.out
    $ ./http_proxy_downoad.out bits-pilani.ac.in 182.75.45.22 13128 csf303 csf303 index.html logo.gif

where the command line arguments are respectively the URL, proxy IP, proxy
port, login, password, filename to save html as, filename to save logo as.

Program works even if the proxy IP changes, or the login/password changes, or BITS changes its logo image, or the base URL is changed.

### Test cases:
1. Correctly downloading the website and image from http://info.in2p3.fr/. 
2. Correct download of HTTP websites and correct handling of HTTP redirects as well, if any.
5. Bonus: handling https for a static https website and handling of redirects, if any (Needs to be done)

The following domains to be accessed through the proxy to test the code:
    
    1. .google.com
    2. .india.gov.in
    3. .bits-pilani.ac.in
    4. .in2p3.fr
    5. .go.com
    6. .jandarshan.cg.nic.in
 
### Use automated perl script for testing
