import json
from zeroconf import ServiceInfo, Zeroconf
import io
import asyncio as asy
import package as pk
import socket
import os
from dotenv import load_dotenv
from quart import Quart, send_file, render_template, jsonify, websocket

app = Quart(__name__)
image_buffer = io.BytesIO()

load_dotenv(".env")

currClient = 0

connected_clients = {}
for i in range(int(os.getenv('MAX_DEVICES'))):
    connected_clients[i] = None

@app.websocket('/ws')
async def ws():
    global currClient
    print("Client connected")
    client_id = -1
    
    try:
        # Cleanup stale connections first
        for i in connected_clients:
            if connected_clients[i] is not None:
                try:
                    await connected_clients[i].ping()
                except:
                    connected_clients[i] = None

        # First assign a client ID before any communication
        for i in connected_clients:
            if connected_clients[i] is None:
                connected_clients[i] = websocket._get_current_object()
                client_id = i
                break
        
        if client_id == -1:  # No slots available
            print("No slots available, rejecting connection")
            return
        
        pk.deprint("Handshake initiated")
        # Send welcome message with timeout
        try:
            await asy.wait_for(
                connected_clients[client_id].send_json({
                    "type": "connection",
                    "message": f"successfully registered ! Hello module {client_id}"
                }),
                timeout=5.0
            )
        except asy.TimeoutError:
            print(f"Handshake timeout for client {client_id}")
            connected_clients[client_id] = None
            return
            
        # Main message loop
        while True:
            try:
                jpack = await asy.wait_for(websocket.receive(), timeout=30.0)
                print("Received data length: ", len(jpack))
                data = json.loads(jpack)
                segment = await pk.manage_request(data, client_id=client_id, currClient=currClient, ori_buffer=image_buffer)
                injson = {"type":"servo", "angle":segment}
                await connected_clients[client_id].send_json(injson)
            except json.JSONDecodeError as je:
                print(f"JSON Error for client {client_id}: {str(je)}")
                continue
            except asy.TimeoutError as te:
                print(f"Timeout for client {client_id}: {str(te)}")
                break
            except Exception as e:
                print(f"Unexpected error for client {client_id}")
                print(f"Error type: {type(e).__name__}")
                print(f"Error message: {str(e)}")
                break

    except Exception as outer_e:
        print(f"Outer error for client {client_id}")
        print(f"Error type: {type(outer_e).__name__}")
        print(f"Error message: {str(outer_e)}")
    
    finally:
        # Cleanup code that will always run
        if client_id != -1:
            connected_clients[client_id] = None
            print(f"Client {client_id} disconnected")


@app.route("/next-module", methods=["POST"])
async def tonextmodule():
    global currClient
    if(currClient+1 > int(os.getenv('MAX_DEVICES'))-1):
        currClient = 0
    else:
        currClient += 1
    return jsonify({"device":currClient})

@app.route("/img", methods=["GET"])
async def pullImageFile():
    global image_buffer
    return await send_file(image_buffer, mimetype='image/jpeg')

@app.route("/", methods=["GET"])
async def servepage():
    return await render_template('index.html')

@app.route("/del", methods=["POST"])
async def delCached():
    print("\nDelete Websocket Client's connections...")
    # Close all active connections
    for client_id in connected_clients:
        if connected_clients[client_id] is not None:
            try:
                await connected_clients[client_id].close()
            except:
                pass
            connected_clients[client_id] = None
    
    # Return a JSON response indicating success
    return jsonify({"status": "success", "message": "All connections cleared"})

async def init_zeroconf():
    # finding local ip
    localIP = socket.gethostbyname(socket.gethostname())
    ipInBytes = socket.inet_aton(localIP)

    # make the mDNS instance
    zcnf = Zeroconf()
    # add the service info
    info = ServiceInfo(
        "_http._tcp.local.",
        "goldentrash._http._tcp.local.",
        addresses=[ipInBytes], 
        port=int(os.getenv('WEBSOCKET_AND_HTTP_PORT')), 
        properties={
            "max_connections":f"{int(os.getenv('MAX_DEVICES'))}", 
            "local_ip":f"{localIP}",
            "description": "local websocket for Golden Trash devices"
        }, 
        server=os.getenv('WEBSOCKET_HOST_NAME')
    )

    await zcnf.async_register_service(info)
    return zcnf, info

async def main():
    await asy.sleep(1)
    zcnf, info = await init_zeroconf()
    
    print(f"WebSocket server registered with mDNS as ws://{os.getenv('WEBSOCKET_HOST_NAME')}:{int(os.getenv('WEBSOCKET_AND_HTTP_PORT'))}/ws")

    try:
        await app.run_task(
            host="0.0.0.0",
            port=int(os.getenv('WEBSOCKET_AND_HTTP_PORT')),
            debug=True
        )
    except KeyboardInterrupt:
        print("\nShutting down server gracefully...")
        # Close all active connections
        for client_id in connected_clients:
            if connected_clients[client_id] is not None:
                try:
                    await connected_clients[client_id].close()
                except:
                    pass
                connected_clients[client_id] = None
    finally:
        await zcnf.async_unregister_service(info)
        await zcnf._async_close()

if __name__ == "__main__":
    print("Starting server ...")
    print(f"Server running at http://localhost:{int(os.getenv('WEBSOCKET_AND_HTTP_PORT'))}/")
    asy.run(main())
