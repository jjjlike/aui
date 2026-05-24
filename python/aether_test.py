#!/usr/bin/env python3
"""
Aether Test Client Library

Python client for interacting with Aether Logic Layer via RPC.
"""

import socket
import json
import time
from typing import Optional, Dict, List, Any


class AetherClient:
    """Client for Aether UI Engine Logic Layer"""
    
    def __init__(self, host: str = "localhost", port: int = 50051):
        self.host = host
        self.port = port
        self.socket = None
        
    def connect(self):
        """Connect to the RPC server"""
        if self.socket:
            self.disconnect()
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.host, self.port))
        
    def disconnect(self):
        """Disconnect from the RPC server"""
        if self.socket:
            self.socket.close()
            self.socket = None
            
    def _send_request(self, request: str) -> str:
        """Send request to server and get response"""
        if not self.socket:
            self.connect()
            
        self.socket.sendall(request.encode('utf-8'))
        response = self.socket.recv(8192).decode('utf-8')
        return response
        
    def get_component_tree(self) -> Dict[str, Any]:
        """Get the component tree as JSON"""
        response = self._send_request("getComponentTree")
        return json.loads(response)
        
    def get_property(self, component_id: str, prop: str) -> Any:
        """Get a property value of a component"""
        request = f"getProperty {component_id} {prop}"
        response = self._send_request(request)
        try:
            return json.loads(response)
        except:
            return response.strip('"')
            
    def set_property(self, component_id: str, prop: str, value: Any) -> bool:
        """Set a property value of a component"""
        if isinstance(value, str):
            value_json = f'"{value}"'
        else:
            value_json = json.dumps(value)
            
        request = f'setProperty {component_id} {prop} {value_json}'
        response = self._send_request(request)
        result = json.loads(response)
        return result.get('success', False)
        
    def inject_mouse_event(self, event_type: str, x: int, y: int, button: int = 0) -> bool:
        """Inject a mouse event"""
        request = f"injectMouseEvent {event_type} {x} {y} {button}"
        response = self._send_request(request)
        result = json.loads(response)
        return result.get('success', False)
        
    def inject_key_event(self, event_type: str, key_code: int, modifiers: int = 0) -> bool:
        """Inject a keyboard event"""
        request = f"injectKeyEvent {event_type} {key_code} {modifiers}"
        response = self._send_request(request)
        result = json.loads(response)
        return result.get('success', False)
        
    def inject_text_input(self, text: str) -> bool:
        """Inject text input"""
        request = f'injectTextInput {text}'
        response = self._send_request(request)
        result = json.loads(response)
        return result.get('success', False)
        
    def take_snapshot(self) -> str:
        """Take a snapshot of the current state"""
        response = self._send_request("takeSnapshot")
        return response
        
    def wait_for_condition(self, path: str, expected: str, timeout_ms: int = 5000) -> Dict[str, Any]:
        """Wait for a condition to be satisfied"""
        request = f"waitForCondition {path} {expected} {timeout_ms}"
        response = self._send_request(request)
        return json.loads(response)
        
    def get_event_log(self) -> List[str]:
        """Get the event log"""
        response = self._send_request("getEventLog")
        return json.loads(response)
        
    def advance_time(self, ms: int) -> bool:
        """Advance the simulation time"""
        request = f"advanceTime {ms}"
        response = self._send_request(request)
        result = json.loads(response)
        return result.get('success', False)
        
    def click(self, x: int, y: int, button: int = 0) -> bool:
        """Simulate a mouse click at the specified coordinates"""
        return self.inject_mouse_event("click", x, y, button)
        
    def type_text(self, text: str) -> bool:
        """Simulate typing text"""
        success = True
        for char in text:
            success = success and self.inject_text_input(char)
        return success


class EventRecorder:
    """Record and playback events"""
    
    def __init__(self, client: AetherClient):
        self.client = client
        self.events = []
        
    def record(self, duration_ms: int):
        """Record events for the specified duration"""
        start_time = time.time() * 1000
        while (time.time() * 1000 - start_time) < duration_ms:
            time.sleep(0.1)
            
    def save_recording(self, filename: str):
        """Save the recording to a file"""
        with open(filename, 'w') as f:
            json.dump(self.events, f, indent=2)
            
    def load_recording(self, filename: str):
        """Load a recording from a file"""
        with open(filename, 'r') as f:
            self.events = json.load(f)
            
    def playback(self):
        """Playback the recorded events"""
        for event in self.events:
            event_type = event.get('type')
            if event_type == 'mouse':
                self.client.inject_mouse_event(
                    event['data']['type'],
                    event['data']['x'],
                    event['data']['y'],
                    event['data'].get('button', 0)
                )
            elif event_type == 'key':
                self.client.inject_key_event(
                    event['data']['type'],
                    event['data']['key_code'],
                    event['data'].get('modifiers', 0)
                )
            elif event_type == 'text':
                self.client.inject_text_input(event['data']['text'])


def example_test():
    """Example test script"""
    print("Connecting to Aether Logic Server...")
    client = AetherClient("localhost", 50051)
    
    try:
        client.connect()
        print("Connected successfully!")
        
        print("\nGetting component tree...")
        tree = client.get_component_tree()
        print(json.dumps(tree, indent=2))
        
        print("\nTaking snapshot...")
        snapshot = client.take_snapshot()
        print(snapshot)
        
        print("\nTest completed successfully!")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        client.disconnect()


if __name__ == "__main__":
    example_test()
