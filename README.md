# Web Server in C

A simple HTTP server written in **C**, using **OpenSSL** to encrypt communication over HTTPS.  
This project is part of my journey to master low-level networking and security while showcasing my passion for the C language.

---

## 🚀 Current Features

- ✅ Non-blocking TLS server (using OpenSSL)
- ✅ Basic request and response handling
- ✅ Serves a small website over HTTPS

---

## 🧠 Goals / TODOs

- [ ] **Properly handle `SIGPIPE`**  
  Prevent server crashes due to broken pipe errors (e.g., when a client disconnects prematurely).

- [ ] **Fully implement HTTP/1.1 protocol**  
  Handle persistent connections, chunked transfers, proper headers, and status codes.

- [ ] **Support HTTP methods beyond GET**  
  Add support for POST, PUT, DELETE, etc.
