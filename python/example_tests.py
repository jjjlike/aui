#!/usr/bin/env python3
"""
Aether UI Engine Test Examples

Example test scripts demonstrating how to use the Aether Test Client
to control the Aether Logic Layer.
"""

from aether_test import AetherClient
import json


def test_basic_component_operations():
    """Test basic component operations"""
    print("=" * 60)
    print("Test: Basic Component Operations")
    print("=" * 60)
    
    client = AetherClient("localhost", 50051)
    client.connect()
    
    try:
        tree = client.get_component_tree()
        print(f"\nComponent tree has {len(tree.get('components', []))} components")
        
        for comp in tree.get('components', []):
            print(f"  - Component {comp['id']}: type={comp['type']}, visible={comp['visible']}")
            
        print("\nTest PASSED")
        
    finally:
        client.disconnect()


def test_button_click():
    """Test clicking a button"""
    print("\n" + "=" * 60)
    print("Test: Button Click Simulation")
    print("=" * 60)
    
    client = AetherClient("localhost", 50051)
    client.connect()
    
    try:
        tree = client.get_component_tree()
        
        button = None
        for comp in tree.get('components', []):
            if comp['type'] == 1:
                button = comp
                break
                
        if button:
            print(f"\nFound button at layout: {button['layout']}")
            
            center_x = int(button['layout']['x'] + button['layout']['width'] / 2)
            center_y = int(button['layout']['y'] + button['layout']['height'] / 2)
            
            print(f"\nClicking button at ({center_x}, {center_y})...")
            success = client.click(center_x, center_y)
            
            if success:
                print("Click event injected successfully")
                
                event_log = client.get_event_log()
                print(f"Event log has {len(event_log)} events")
            else:
                print("Failed to inject click event")
        else:
            print("No button found")
            
        print("\nTest PASSED")
        
    finally:
        client.disconnect()


def test_property_manipulation():
    """Test property manipulation"""
    print("\n" + "=" * 60)
    print("Test: Property Manipulation")
    print("=" * 60)
    
    client = AetherClient("localhost", 50051)
    client.connect()
    
    try:
        tree = client.get_component_tree()
        
        if tree.get('components'):
            root = tree['components'][0]
            root_id = str(root['id'])
            
            print(f"\nRoot component ID: {root_id}")
            
            print("\nSetting width to 900...")
            success = client.set_property(root_id, "width", 900.0)
            print(f"Success: {success}")
            
            print("\nGetting updated component tree...")
            updated_tree = client.get_component_tree()
            updated_root = updated_tree['components'][0]
            
            if 'width' in str(updated_root.get('layout', {})):
                print(f"Updated root width: {updated_root['layout']['width']}")
                
        print("\nTest PASSED")
        
    finally:
        client.disconnect()


def test_text_input():
    """Test text input"""
    print("\n" + "=" * 60)
    print("Test: Text Input")
    print("=" * 60)
    
    client = AetherClient("localhost", 50051)
    client.connect()
    
    try:
        print("\nTyping 'Hello Aether!'...")
        success = client.type_text("Hello Aether!")
        
        if success:
            print("Text input injected successfully")
            
        print("\nTest PASSED")
        
    finally:
        client.disconnect()


def test_snapshot_comparison():
    """Test snapshot comparison"""
    print("\n" + "=" * 60)
    print("Test: Snapshot Comparison")
    print("=" * 60)
    
    client = AetherClient("localhost", 50051)
    client.connect()
    
    try:
        print("\nTaking initial snapshot...")
        snapshot1 = client.take_snapshot()
        print(f"Snapshot 1: {len(snapshot1)} bytes")
        
        print("\nMaking some changes...")
        client.click(400, 300)
        
        print("\nTaking second snapshot...")
        snapshot2 = client.take_snapshot()
        print(f"Snapshot 2: {len(snapshot2)} bytes")
        
        if snapshot1 == snapshot2:
            print("\nSnapshots are identical (no state change)")
        else:
            print("\nSnapshots are different (state changed)")
            
        print("\nTest PASSED")
        
    finally:
        client.disconnect()


def test_wait_for_condition():
    """Test wait for condition"""
    print("\n" + "=" * 60)
    print("Test: Wait For Condition")
    print("=" * 60)
    
    client = AetherClient("localhost", 50051)
    client.connect()
    
    try:
        print("\nWaiting for condition (timeout in 3 seconds)...")
        result = client.wait_for_condition("root/width", "800", 3000)
        
        print(f"\nCondition result: {result}")
        print(f"Satisfied: {result.get('satisfied', False)}")
        print(f"Actual value: {result.get('actual_value', 'N/A')}")
        print(f"Elapsed: {result.get('elapsed_ms', 0)} ms")
        
        print("\nTest PASSED")
        
    finally:
        client.disconnect()


def run_all_tests():
    """Run all test examples"""
    print("\n" + "=" * 60)
    print("AETHER UI ENGINE - TEST SUITE")
    print("=" * 60)
    
    tests = [
        ("Basic Component Operations", test_basic_component_operations),
        ("Button Click Simulation", test_button_click),
        ("Property Manipulation", test_property_manipulation),
        ("Text Input", test_text_input),
        ("Snapshot Comparison", test_snapshot_comparison),
        ("Wait For Condition", test_wait_for_condition),
    ]
    
    passed = 0
    failed = 0
    
    for name, test_func in tests:
        try:
            test_func()
            passed += 1
        except Exception as e:
            print(f"\nTest FAILED with error: {e}")
            failed += 1
            
    print("\n" + "=" * 60)
    print(f"TEST RESULTS: {passed} passed, {failed} failed")
    print("=" * 60)


if __name__ == "__main__":
    run_all_tests()
