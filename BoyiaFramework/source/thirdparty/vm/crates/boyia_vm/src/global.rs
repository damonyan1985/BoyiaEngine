//! Doubly linked list based on [Global]. Each node has [Global::prev] and [Global::next]
//! pointing to previous and next node, and [Global::value] holding a [BoyiaValue].

use crate::BoyiaValue;
use std::ptr::null_mut;

/// Node of a doubly linked list. Use [Global::prev], [Global::next], [Global::value] to access fields.
pub struct Global {
    prev: *mut Global,
    next: *mut Global,
    value: BoyiaValue,
    /// Owns the next node so that [prev]/[next] raw pointers remain valid.
    next_owner: Option<Box<Global>>,
}

impl Global {
    /// New node with no links. [prev] and [next] are null.
    pub fn new(value: BoyiaValue) -> Self {
        Global {
            prev: null_mut(),
            next: null_mut(),
            value,
            next_owner: None,
        }
    }

    /// Previous node pointer, or null if none.
    #[inline]
    pub fn prev(&self) -> *mut Global {
        self.prev
    }

    /// Next node pointer, or null if none.
    #[inline]
    pub fn next(&self) -> *mut Global {
        self.next
    }

    /// The stored [BoyiaValue].
    #[inline]
    pub fn value(&self) -> BoyiaValue {
        self.value
    }

    /// Pointer to the stored [BoyiaValue] (for GC marking). The pointer is valid while this [Global] is alive.
    #[inline]
    pub fn value_ptr(&self) -> *mut BoyiaValue {
        &self.value as *const BoyiaValue as *mut BoyiaValue
    }
}

/// Doubly linked list of [Global] nodes.
pub struct GlobalList {
    head: Option<Box<Global>>,
    tail: *mut Global,
}

impl GlobalList {
    pub fn new() -> Self {
        GlobalList {
            head: None,
            tail: null_mut(),
        }
    }

    /// Append a [BoyiaValue] at the back. Returns a raw pointer to the new node.
    pub fn push_back(&mut self, value: BoyiaValue) -> *mut Global {
        let mut node = Box::new(Global::new(value));
        let ptr = node.as_mut() as *mut Global;

        if self.tail.is_null() {
            self.head = Some(node);
            self.tail = ptr;
        } else {
            unsafe {
                (*self.tail).next = ptr;
                (*self.tail).next_owner = Some(node);
            }
            unsafe {
                (*ptr).prev = self.tail;
            }
            self.tail = ptr;
        }
        ptr
    }

    /// Prepend a [BoyiaValue] at the front. Returns a raw pointer to the new node.
    pub fn push_front(&mut self, value: BoyiaValue) -> *mut Global {
        let mut new_box = Box::new(Global::new(value));
        let ptr = new_box.as_mut() as *mut Global;

        if let Some(mut old_head) = self.head.take() {
            let old_ptr = old_head.as_ref() as *const Global as *mut Global;
            old_head.as_mut().prev = ptr;
            new_box.next = old_ptr;
            new_box.next_owner = Some(old_head);
            self.head = Some(new_box);
        } else {
            self.tail = ptr;
            self.head = Some(new_box);
        }
        ptr
    }

    /// Head node pointer, or null if empty.
    pub fn head(&self) -> *mut Global {
        self.head
            .as_ref()
            .map(|b| b.as_ref() as *const Global as *mut Global)
            .unwrap_or(null_mut())
    }

    /// Tail node pointer, or null if empty.
    pub fn tail(&self) -> *mut Global {
        self.tail
    }

    /// Remove the node at the given pointer from the list. No-op if ptr is null or not in this list.
    pub fn remove(&mut self, ptr: *mut Global) {
        if ptr.is_null() {
            return;
        }
        unsafe {
            let prev = (*ptr).prev;
            let next = (*ptr).next;
            let next_owner = std::mem::take(&mut (*ptr).next_owner);

            if prev.is_null() {
                // Removing head: new head is the next node (owned by our next_owner).
                if self.head.as_ref().map(|b| b.as_ref() as *const Global) == Some(ptr) {
                    self.head = next_owner;
                    if let Some(ref mut new_head) = self.head {
                        new_head.prev = null_mut();
                    } else {
                        self.tail = null_mut();
                    }
                }
            } else {
                (*prev).next = next;
                (*prev).next_owner = next_owner;
            }

            if next.is_null() {
                self.tail = prev;
            } else {
                (*next).prev = prev;
            }
        }
    }
}

impl Default for GlobalList {
    fn default() -> Self {
        Self::new()
    }
}
