import threading
import requests 
import os

urls2 = [
"http://127.0.0.1:8080/captcha.xyz?",
"http://[::1]:8080/captcha.xyz?",
"https://127.0.0.1:8081/captcha.xyz?",
"https://[::1]:8081/captcha.xyz?",
"http://127.0.0.1:8080/p3.xyz?a=1&b=2&c=3",
"http://[::1]:8080/p3.xyz?a=1&b=2&c=3",
"https://127.0.0.1:8081/p3.xyz?a=1&b=2&c=3",
"https://[::1]:8081/p3.xyz?a=1&b=2&c=3",
]

urls = [
    "http://127.0.0.1:8080/login.html",
    "http://[::1]:8080/login.html",
    "https://127.0.0.1:8081/login.html",
    "https://[::1]:8081/login.html",
    "http://127.0.0.1:8080/nandihills.jpg",
    "http://[::1]:8080/nandihills.jpg",
    "https://127.0.0.1:8081/nandihills.jpg",
    "https://[::1]:8081/nandihills.jpg",
    "http://127.0.0.1:8080/nandihills.jpg",
    "http://[::1]:8080/nandihills.jpg",
    "https://127.0.0.1:8081/nandihills.jpg",
    "https://[::1]:8081/nandihills.jpg",
    "http://127.0.0.1:8080/image.jpg",
    "http://[::1]:8080/image.jpg",
    "https://127.0.0.1:8081/image.jpg",
    "https://[::1]:8081/image.jpg",
    "http://127.0.0.1:8080/multipart.html",
    "http://[::1]:8080/multipart.html",
    "https://127.0.0.1:8081/multipart.html",
    "https://[::1]:8081/multipart.html",
    "http://127.0.0.1:8080/multipart2.html",
    "http://[::1]:8080/multipart2.html",
    "https://127.0.0.1:8081/multipart2.html",
    "https://[::1]:8081/multipart2.html",
#    "http://127.0.0.1:8080/large.jpg",
#    "http://[::1]:8080/large.jpg",
#    "https://127.0.0.1:8081/large.jpg",
#    "https://[::1]:8081/large.jpg",
    "http://127.0.0.1:8080/large1.jpg",
    "http://[::1]:8080/large1.jpg",
    "https://127.0.0.1:8081/large1.jpg",
    "https://[::1]:8081/large1.jpg",
    "http://127.0.0.1:8080/router/embedimages/d1.jpeg",
    "http://[::1]:8080/router/embedimages/d1.jpeg",
    "https://127.0.0.1:8081/router/embedimages/d1.jpeg",
    "https://[::1]:8081/router/embedimages/d1.jpeg",
    "http://127.0.0.1:8080/router/embedimages/d2.jpeg",
    "http://[::1]:8080/router/embedimages/d2.jpeg",
    "https://127.0.0.1:8081/router/embedimages/d2.jpeg",
    "https://[::1]:8081/router/embedimages/d2.jpeg",
    "http://127.0.0.1:8080/router/embedimages/d3.jpeg",
    "http://[::1]:8080/router/embedimages/d3.jpeg",
    "https://127.0.0.1:8081/router/embedimages/d3.jpeg",
    "https://[::1]:8081/router/embedimages/d3.jpeg",
    "http://127.0.0.1:8080/router/embedimages/d4.jpeg",
    "http://[::1]:8080/router/embedimages/d4.jpeg",
    "https://127.0.0.1:8081/router/embedimages/d4.jpeg",
    "https://[::1]:8081/router/embedimages/d4.jpeg",
    "http://127.0.0.1:8080/router/embedimages/d5.jpeg",
    "http://[::1]:8080/router/embedimages/d5.jpeg",
    "https://127.0.0.1:8081/router/embedimages/d5.jpeg",
    "https://[::1]:8081/router/embedimages/d5.jpeg",
]


lock = threading.Lock()
id = 0


def thread_task():
    i = 0
    j = 0
    k = 0
    global id
    with lock:
        j = id
        id += 1
    while ( i < 10000 ):
        for url in urls:
            k+=1
            try:
                req = requests.get(url, stream = True, verify=False) 
                outfile = "test___" + str(j) + "___" +  str(i) + "___" + str(k)
                print( url + "  --->  " + outfile )
                with open(outfile,"wb") as fhandle: 
                    for chunk in req.iter_content(chunk_size=8192): 
                        if chunk: 
                            fhandle.write(chunk)
                os.remove( outfile )
            except Exception as e :
                pass
        i += 1	


threads = [threading.Thread(target=thread_task) for _ in range(40)]

for thread in threads:
    thread.start()

for thread in threads:
    thread.join()
