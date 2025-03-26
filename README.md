# Web Server in C

A simple HTTP server written in **C**, using **OpenSSL** to encrypt communication over HTTPS.  
This project is part of my journey to master low-level networking and security while showcasing my passion for the C language.

---

## 🚀 Current Features

- ✅ Non-blocking TLS server (using OpenSSL)
- ✅ Basic request and response handling
- ✅ Serves a small website over HTTPS
- ✅ keep alive implemented

---

## 🔐 How to Enable HTTPS

To secure the server with HTTPS, you’ll need an **SSL/TLS certificate** and a **private key**.

### 📥 Obtaining a Certificate

You can obtain a free certificate from a trusted Certificate Authority (CA) such as [Let's Encrypt].

The recommended way to get a certificate from Let's Encrypt is by using an [ACME client] like `certbot`.

[Let's Encrypt]: (https://letsencrypt.org/getting-started)
[ACME client]: (https://certbot.eff.org)

---

### 🔧 Updating the Code

Once you’ve obtained your certificate and private key:

1. Open the file [`com.c`](src/com.c)  
2. Modify the hardcoded certificate and key paths near **line 155** to match the location of your `.crt` and `.key` files.
3. Rebuild the server using:


## 🧾 Makefile Rules

The Makefile provides two main build targets, each creating a different binary with distinct purposes:

### 🔍 `make build` – Debug/Development Build

Builds the full `server` binary with **AddressSanitizer (ASan)** enabled.  
This version is useful for:

- Detecting memory leaks
- Catching buffer overflows
- Improving overall code safety during development

**Note**: ASan introduces additional memory and performance overhead. It's not recommended for production use.

- Output binary: `./server`

### ⚡ `make light` – Lightweight Production Build

Builds a slimmer `web_server` binary without ASan or extra development tools.  
This version is suitable for deployment where performance and lower memory usage are important.

- Output binary: `./web_server`

---

### 📌 Summary

| Command       | Output        | Purpose                     |
|---------------|---------------|-----------------------------|
| `make build`  | `./server`    | Full build with ASan (debug) |
| `make light`  | `./web_server`| Lightweight build (release)  |



##  TODOs

- [ ] **Properly handle `SIGPIPE`**  
  Prevent server crashes due to broken pipe errors (e.g., when a client disconnects prematurely).

- [ ] **Fully implement HTTP/1.1 protocol**  
  Handle persistent connections, chunked transfers, proper headers, and status codes.

- [ ] **Support HTTP methods beyond GET**  
  Add support for POST, PUT, DELETE, etc.
