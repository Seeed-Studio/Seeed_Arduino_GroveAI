/***
 * lwRingBuffer.hpp
 * Description: A light-weight ring buffer.
 * 2023 Copyright (c) Seeed Technology Inc.  All right reserved.
 * Author: Jiaxuan Weng(jiaxuan.weng@outlook.com)
 * Copyright (C) 2023  Seeed Technology Co.,Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


class lwRingBuffer {
public:
    lwRingBuffer(size_t len) : buf(new char[len]), len(len), head(0), tail(0) {}
    ~lwRingBuffer() {
        delete[] buf;
    }
    void push(char c) {
        if (isFull()) {
            head = (head + 1) % len;
        }
        buf[tail] = c;
        tail = (tail + 1) % len;
    }
    void insert(char c) { this->push(c); }
    void put(char c) { this->push(c); }

    char pop() {
        if (isEmpty()) {
            return 0;
        }
        char c = buf[head];
        head = (head + 1) % len;
        return c;
    }
    char remove() { return this->pop(); }
    char get() { return this->pop(); }

    char peek() {
        if (isEmpty()) {
            return 0;
        }
        return buf[head];
    }
    bool isEmpty() {
        return head == tail;
    }
    bool isFull() {
        return (tail + 1) % len == head;
    }
    size_t size() {
        return (tail - head + len) % len;
    }
    size_t capacity() {
        return len;
    }
    void clear() {
        head = tail;
    }

    friend lwRingBuffer& operator>>(lwRingBuffer& input, char &c) {
        c = input.pop();
        return input;
    }
    friend lwRingBuffer& operator<<(lwRingBuffer& output, char c) {
        output.push(c);
        return output;
    }
    char operator[](int i) {
        return buf[(head + i) % len];
    }
    int find(char c) {
        for (int i = head; i != tail; i = (i + 1) % len) {
            if (buf[i] == c) {
                return (i - head + len) % len;
            }
        }
        return -1;
    }
    bool match(const char* str, int slen) {
        if (slen > size()) {
            return false;
        }
        for (int i = 0; i < slen; i++) {
            if (buf[(head + i) % len] != str[i]) {
                return false;
            }
        }
        return true;
    }
    int extract(char c, char* str, int slen) {
        int i = find(c);
        if (i == -1) {
            return 0;
        }
        if (slen < i) {
            head = (head + i + 1) % len;
            return 0;
        }
        for (int j = 0; j <= i; j++) {
            str[j] = buf[(head + j) % len];
        }
        head = (head + i + 1) % len;
        return i + 1;
    }

private:
    char *buf;
    size_t len;
    int head;
    int tail;
};
