import threading

lock = threading.Lock()
counter = 0

def worker(name: str) -> None:
    global counter

    for _ in range(5):
        with lock:
            counter += 1
            print(f"{name} incremented counter to {counter}")

t1 = threading.Thread(target=worker, args=("Thread 1",))
t2 = threading.Thread(target=worker, args=("Thread 2",))

t1.start()
t2.start()

t1.join()
t2.join()

print("Final counter value:", counter)