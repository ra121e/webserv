#!/usr/bin/env python3
"""
CGI script to handle file uploads via multipart/form-data POST.
Saves the uploaded file into the server's tmp directory.
"""
import os, sys, html, re, traceback

# Utility functions

def send_response(body, status_code=200, status_text='OK', content_type='text/html; charset=utf-8'):
    sys.stdout.write(f"HTTP/1.1 {status_code} {status_text}\r\n")
    sys.stdout.write(f"Content-Type: {content_type}\r\n\r\n")
    sys.stdout.write(body)


def error_page(message, code=400, title='Upload Error'):
    escaped = html.escape(message)
    body = f"""<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>{title}</title>
<style>
body {{ background:#f8f8f8; font-family:Arial,sans-serif; margin:0; padding:0; text-align:center; color:#333; }}
.container {{ max-width:700px; margin:80px auto; padding:20px; }}
h1 {{ font-size:64px; margin:0; color:#c0392b; }}
p {{ font-size:18px; color:#555; }}
a.button {{ display:inline-block; padding:10px 18px; background:#3498db; color:#fff; text-decoration:none; border-radius:5px; font-weight:bold; }}
a.button:hover {{ background:#2980b9; }}
.topbar {{ background:#111827; color:#e5e7eb; padding:10px 16px; text-align:left; }}
.topbar a {{ color:#e5e7eb; text-decoration:none; margin-right:12px; font-weight:600; }}
.topbar a:hover {{ color:#38bdf8; }}
code {{ background:#eee; padding:2px 4px; border-radius:3px; }}
</style></head><body>
<div class='topbar'>
  <a href='/'>Home</a>
  <a href='/about'>About</a>
  <a href='/status'>Status</a>
  <a href='/upload'>Upload/Delete</a>
  <a href='/uploads'>Uploads</a>
</div>
<div class='container'>
  <h1>{code}</h1>
  <h2>{title}</h2>
  <p>{escaped}</p>
  <a class='button' href='/upload'>Try Again</a>
</div></body></html>"""
    send_response(body, status_code=code, status_text=title)


def sanitize_filename(filename: str) -> str:
    # Strip directory components
    filename = filename.split('/')[-1].split('\\')[-1]
    # Allow only safe chars
    filename = re.sub(r'[^A-Za-z0-9._-]', '_', filename)
    # Avoid empty
    if not filename:
        filename = 'upload.bin'
    return filename


def parse_multipart(body: bytes, boundary: bytes):
    # boundary comes without leading '--'; we add for splitting logic
    if not boundary.startswith(b'--'):
        boundary_bytes = b'--' + boundary
    else:
        boundary_bytes = boundary
    # Split parts
    parts = body.split(boundary_bytes)
    files = []
    for part in parts:
        if not part or part in (b'--', b'--\r\n'):
            continue
        # Remove possible leading CRLF
        if part.startswith(b'\r\n'):
            part = part[2:]
        # Detect final boundary termination
        if part.endswith(b'--\r\n'):
            part = part[:-4]
        elif part.endswith(b'--'):
            part = part[:-2]
        # Separate headers and data
        header_end = part.find(b'\r\n\r\n')
        if header_end == -1:
            continue
        header_block = part[:header_end].decode('utf-8', 'replace')
        data = part[header_end+4:]
        # Trim trailing CRLF typical at end
        if data.endswith(b'\r\n'):
            data = data[:-2]
        headers = {}
        for line in header_block.split('\r\n'):
            if ':' in line:
                k,v = line.split(':',1)
                headers[k.strip().lower()] = v.strip()
        disp = headers.get('content-disposition','')
        # Parse content-disposition
        m = re.search(r'filename="(.*?)"', disp)
        if m:
            raw_filename = m.group(1)
            filename = sanitize_filename(raw_filename)
            files.append({'filename': filename, 'data': data, 'headers': headers})
    return files


def main():
    method = os.environ.get('REQUEST_METHOD','').upper()
    if method != 'POST':
        error_page('Only POST method is supported for uploads.', 405, 'Method Not Allowed')
        return

    content_type = os.environ.get('CONTENT_TYPE','')
    if 'multipart/form-data' not in content_type:
        error_page('Content-Type must be multipart/form-data.', 400, 'Bad Request')
        return

    content_length = os.environ.get('CONTENT_LENGTH')
    if not content_length:
        error_page('Missing Content-Length header.', 411, 'Length Required')
        return
    try:
        length = int(content_length)
    except ValueError:
        error_page('Invalid Content-Length.', 400, 'Bad Request')
        return

    try:
        body = sys.stdin.buffer.read(length)
    except Exception as e:
        error_page(f'Failed reading request body: {e}', 500, 'Internal Server Error')
        return

    # Extract boundary
    m = re.search(r'boundary=([^;]+)', content_type)
    if not m:
        error_page('Missing multipart boundary.', 400, 'Bad Request')
        return
    boundary = m.group(1).encode('utf-8')

    try:
        files = parse_multipart(body, boundary)
    except Exception as e:
        error_page(f'Failed parsing multipart data: {html.escape(str(e))}', 400, 'Bad Request')
        return

    if not files:
        error_page('No file parts found in upload.', 400, 'Bad Request')
        return

    # Determine tmp directory path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    tmp_dir = os.path.realpath(os.path.join(script_dir, '..', 'tmp'))
    if not os.path.isdir(tmp_dir):
        try:
            os.makedirs(tmp_dir, exist_ok=True)
        except Exception as e:
            error_page(f'Failed to create tmp directory: {e}', 500, 'Internal Server Error')
            return

    saved = []
    for f in files:
        filepath = os.path.join(tmp_dir, f['filename'])
        # Avoid overwriting by adding suffix if exists
        base, ext = os.path.splitext(filepath)
        counter = 1
        while os.path.exists(filepath):
            filepath = f"{base}_{counter}{ext}"
            counter += 1
        try:
            with open(filepath, 'wb') as out:
                out.write(f['data'])
            saved.append((f['filename'], filepath, len(f['data'])))
        except Exception as e:
            error_page(f'Failed saving file {f["filename"]}: {e}', 500, 'Internal Server Error')
            return

    rows = ''.join(f"<tr><td>{html.escape(name)}</td><td>{size} bytes</td><td><code>{html.escape(os.path.relpath(path, tmp_dir))}</code></td></tr>" for name, path, size in saved)

    body = f"""<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>Upload Success</title>
<style>
body {{ background:#f8f8f8; font-family:Arial,sans-serif; margin:0; padding:0; text-align:center; color:#333; }}
.container {{ max-width:800px; margin:60px auto; padding:20px; }}
h1 {{ font-size:48px; margin:0; color:#2e7d32; }}
p {{ font-size:18px; color:#555; }}
a.button {{ display:inline-block; padding:10px 18px; background:#3498db; color:#fff; text-decoration:none; border-radius:5px; font-weight:bold; }}
a.button:hover {{ background:#2980b9; }}
.topbar {{ background:#111827; color:#e5e7eb; padding:10px 16px; text-align:left; }}
.topbar a {{ color:#e5e7eb; text-decoration:none; margin-right:12px; font-weight:600; }}
.topbar a:hover {{ color:#38bdf8; }}
table {{ width:100%; border-collapse:collapse; margin-top:25px; }}
th, td {{ padding:8px 10px; border-bottom:1px solid #ddd; text-align:left; font-size:14px; }}
th {{ background:#eee; }}
</style></head><body>
<div class='topbar'>
  <a href='/'>Home</a>
  <a href='/about'>About</a>
  <a href='/status'>Status</a>
  <a href='/upload'>Upload/Delete</a>
  <a href='/uploads'>Uploads</a>
</div>
<div class='container'>
  <h1>Upload Successful</h1>
  <p>{len(saved)} file(s) uploaded.</p>
  <table><thead><tr><th>Filename</th><th>Size</th><th>Stored As</th></tr></thead><tbody>{rows}</tbody></table>
  <p><a class='button' href='/upload'>Upload Another</a></p>
</div></body></html>"""
    send_response(body, 200, 'OK')

if __name__ == '__main__':
    try:
        main()
    except Exception:
        # Fallback fatal error
        tb = html.escape(traceback.format_exc())
        send_response(f"<h1>500 Internal Server Error</h1><pre>{tb}</pre>", 500, 'Internal Server Error')
