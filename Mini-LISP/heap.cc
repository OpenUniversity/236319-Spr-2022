#include "chic.h"
#include "heap.h"

#include "Short.h"
#include "Word.h"
#include "Cons.h"
#include "Pristine.h"

#include "layout.h"

Pristine heapify();
Pristine heap = heapify();

static Knob fresh() {
  Expect(!heap.x());
  Pristine::count--;
  let __ = Pristine(heap).prev($P_x$);
  heap = __.next();
  return __.inner(); 
}

Item fresh(Short s1, Short s2) {
  Expect(!marked(s1));
  Expect(marked(s1));
  if (heap.x()) throw __LINE__; 
  return fresh().Item().head(s1).rest(s2);
}

static Item fresh(Word w) { 
  return fresh(w.s1,w.s2); 
}

static Cons require(Word w, Short s) {
  Expect(Cons::ok(w));
  Expect(s >= $P_f$);
  Expect(s <= $P_t$);
  if (P[s].l == w.l) return s; 
  if (!Pristine(s).ok()) return fresh().Cons().car(w.s1).cdr(w.s2);
  Promise(Cons::ok(P[s]));
  auto prev = Pristine(s).prev(), next = Pristine(s).next();
  if (!prev.x()) prev.next(next); 
  if (!next.x()) next.prev(prev); 
  if (heap.inner() == s) heap = next ;
  P[s] = w;
  return Cons(s);
}

Cons require(Word w) {
  return require(w, w.hash());
}
  
Pristine heapify() {
  for (Short s = $P_f$ + 1; s <= $P_t$ - 1; ++s) 
    Knob(s).s1(s-1).s2(s+1);
  Knob($P_f$).s1($P_x$).s2($P_f$ +1);
  Knob($P_t$).s2($P_x$).s2($P_t$ -1);
  for (Short s = $P_f$; s <= $P_t$; ++s)  {
    stain(P[s].s1);
    stain(P[s].s2);
  }
  Pristine::count = $P_n$;
  return heap = Pristine($P_f$);
}

Short error() {
  Short __ = 0;
  for (auto h = heap, h2 = h; !h.x(); h = h.next()) { 
    ++__;
    if (!Pristine(h).ok()) return __; 
    if (!h2.x()) h2 = h2.next();
    if (!h2.x()) h2 = h2.next();
    if (h.inner() == h2.inner()) return -__; 
  } 
  return 0;
}

void free(Item i) {
  let h = i.inner();
  Expect(Knob(h).item());
  Pristine(h).prev($P_x$).next(heap); 
  heap = h;
  Pristine::count--;
}

Cons require(Sx car, Sx cdr) { return require(Word(car.inner(),cdr.inner())); }

Short length() {
  Short result = 0;
  for (auto h = heap; !h.x();  h = h.next()) // printf("%d ", h), 
      ++result;
  return result;
}


#include <stdio.h>
#undef Type
#undef function

#include "gtest/gtest.h"

TEST(fresh, Exhausted) { 
  for (;;)
    fresh();
}


TEST(Mark, FixedPoint) { 
  for (auto h = -32768; h != 32767; ++h)
    EXPECT_NE(h, mark(h)) << h;
}

TEST(Mark, Last) {
  EXPECT_FALSE(marked($P_t$ + 1));
}

TEST(Word, hash) { 
  EXPECT_NE(Word(3,4).hash(), Word(4,3).hash());
  EXPECT_NE(Word(3,4).hash(), Word(4,3).hash());
  EXPECT_NE(Word(2,3).hash(), Word(4,5).hash());
  EXPECT_NE(Word(2,3).hash(), Word(4,5).hash());
  EXPECT_GT(Word(2,3).hash(), 4);
  EXPECT_GT(Word(3,4).hash(), 3);
  EXPECT_GT(Word(4,5).hash(), 4);
}

TEST(Heapify, exists) { 
  try {
    heapify();
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Heapify, error) { 
  try {
    heapify();
    EXPECT_EQ(error(), 0);
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Heapify, length) { 
  try {
    heapify();
    error();
    EXPECT_EQ(length(), $P_n$);
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Heapify, Pristine) { 
  heapify();
  EXPECT_FALSE(heap.x());
  EXPECT_EQ(heap.inner(), 1);
  EXPECT_EQ(heap.inner(), $P_f$);
  EXPECT_EQ(Pristine(1).next().inner(), 2);
  EXPECT_EQ(Pristine(1).prev().inner(), 0);
  EXPECT_EQ(Pristine(2).next().inner(), 3);
  EXPECT_EQ(Pristine(2).prev().inner(), 1);
  EXPECT_EQ(Pristine($P_t$).prev().inner(), $P_t$-1);
  EXPECT_EQ(Pristine($P_t$).next().inner(), $P_x$);
}

TEST(Heapify, AllPristine) { 
  heapify();
  for (Short h = $P_f$; h <= $P_t$; ++h)
    EXPECT_TRUE(Pristine(h).ok()) << h;
}

TEST(Fresh, exists) { 
  heapify();
  fresh(15,21);
}

TEST(Fresh, correct) { 
  heapify();
  auto f = fresh(15,21);
  EXPECT_EQ(f.head(), 15);
  EXPECT_EQ(f.head(), 15);
}

TEST(Fresh, 1) { 
  heapify();
  EXPECT_FALSE(error());
  EXPECT_EQ(heap.inner(),1);
  auto s1 = fresh().inner();
  EXPECT_EQ(heap.inner(),2);
  EXPECT_EQ(s1,1);
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-1);
}

TEST(Fresh, 3) { 
  try {
    heapify();
    let s1 = fresh(2,3);
    let s2 = fresh(4,5);
    let s3 = fresh(6,7);
    free(s2);
    free(s1);
    free(s3);
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Fresh, Length) { 
  try {
    heapify();
    EXPECT_EQ(length(), $P_n$);

    let s1 = fresh(2,3);
    EXPECT_EQ(length(), $P_n$ - 1);

    let s2 = fresh(4,5);
    EXPECT_EQ(length(), $P_n$ - 2);

    let s3 = fresh(6,7);
    EXPECT_EQ(length(), $P_n$ - 3);

    free(s1);
    EXPECT_EQ(heap.inner(), 1);
    EXPECT_EQ(length(), $P_n$ - 2);

    free(s3);
    EXPECT_EQ(heap.inner(), 3);
    EXPECT_EQ(length(), $P_n$ - 1);

    free(s2);
    EXPECT_EQ(heap.inner(), 2);
    EXPECT_EQ(length(), $P_n$);
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Require, exists) { 
  try {
    heapify();
    require(Sx(0xDE),Sx(0xAD));
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Require, cons) { 
  try {
    heapify();
    Cons h = require(Sx(0xDE),Sx(0xAD));
    EXPECT_TRUE(h.ok());
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Require, correct) { 
  try {
    heapify();
    Short h = require(Sx(0xDE),Sx(0xAD)).inner();
    EXPECT_EQ(P[h].s1, 0xDE);
    EXPECT_EQ(P[h].s2, 0xAD);
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Require, 3) { 
  try {
    heapify();
    auto s1 = require(2,3);
    auto s2 = require(4,5);
    auto s3 = require(6,7);
  } catch(int e) {
    ADD_FAILURE() << "Died at line " << e;
  }
}

TEST(Require, correct3) { 
  heapify();
  EXPECT_FALSE(error());
  EXPECT_EQ(heap.inner(),1);
  auto s1 = require(2,3);
  EXPECT_EQ(heap.inner(),1);
  EXPECT_NE(s1.inner(),1);
  EXPECT_EQ(P[s1.inner()].s1,2);
  EXPECT_EQ(P[s1.inner()].s2,3);
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-1);
  auto s2 = require(4,5);
  EXPECT_EQ(s2.car().inner(),4);
  EXPECT_EQ(s2.cdr().inner(),5);
  EXPECT_FALSE(error());
  EXPECT_EQ(heap.inner(),1);
  EXPECT_EQ(length(), $P_n$-2);
  auto s3 = require(6,7);
  EXPECT_EQ(heap.inner(),1);
  EXPECT_EQ(s3.car().inner(),6);
  EXPECT_EQ(P[s3.inner()].s2,7);
  EXPECT_EQ(P[s3.inner()].s2,s3.cdr().inner());
}

TEST(Require, Scenario) { 
  heapify();
  EXPECT_FALSE(error());
  EXPECT_EQ(heap.inner(),1);
  auto s1 = require(2,3);
  EXPECT_EQ(heap.inner(),1);
  EXPECT_EQ(s1.inner(),Word(2,3).hash());
  EXPECT_EQ(P[s1.inner()].s1,2);
  EXPECT_EQ(P[s1.inner()].s2,3);
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-1);
  auto s2 = require(4,5);
  EXPECT_EQ(s2.inner(),Word(4,5).hash());
  EXPECT_EQ(heap.inner(),1);
  EXPECT_EQ(P[s2.inner()].s1,4);
  EXPECT_EQ(P[s2.inner()].s2,5);
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-2);
  auto s3 = require(6,7);
  EXPECT_EQ(s3.inner(),Word(6,7).hash());
  EXPECT_EQ(P[s3.inner()].s1,6);
  EXPECT_EQ(P[s3.inner()].s2,7);
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-3);
}

TEST(Words, Reuse) { 
  heapify();
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$);

  auto s1 = require(3,3);
  EXPECT_TRUE(s1.ok());
  EXPECT_EQ(s1.inner(), P[s1.inner()].hash());
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-1);

  auto s2 = require(2,3);
  EXPECT_TRUE(s2.ok());
  EXPECT_EQ(s2.inner(),P[s2.inner()].hash());
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-2);

  auto s3 = require(3,2);
  EXPECT_EQ(s3.inner(), P[s3.inner()].hash());
  EXPECT_TRUE(s3.ok());
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-3);

  auto s4 = require(2,3);
  EXPECT_TRUE(s4.ok());
  EXPECT_EQ(s4.inner(), P[s4.inner()].hash());
  EXPECT_FALSE(error());
  EXPECT_EQ(length(), $P_n$-3);

  auto s5 = require(3,3);
  EXPECT_TRUE(s5.ok());
  EXPECT_EQ(s5.inner(), P[s5.inner()].hash());
  EXPECT_EQ(length(), $P_n$-3);
  EXPECT_FALSE(error());

  auto s6 = require(3,2);
  EXPECT_TRUE(s6.ok());
  EXPECT_EQ(s6.inner(), P[s6.inner()].hash());
  EXPECT_EQ(length(), $P_n$-3);
  EXPECT_FALSE(error());

  EXPECT_EQ(s1.inner(), s5.inner());
  EXPECT_EQ(s2.inner(), s4.inner());
  EXPECT_EQ(s3.inner(), s6.inner());
}

TEST(Marking, Pairs) { 
  EXPECT_LT(mark($P_f$), $A_f$);   
  EXPECT_LT(mark($P_t$), $A_f$);   
  EXPECT_LT(mark(($P_f$ + $P_t$)/2), $A_f$);   
  EXPECT_LT(mark($P_f$-1), $A_f$);   
  EXPECT_LT(mark($P_f$+1), $A_f$);   
  EXPECT_EQ(mark(mark($P_f$)), $P_f$);
  EXPECT_EQ(mark(mark($P_t$)), $P_t$);
  EXPECT_EQ(mark(mark(($P_f$ + $P_t$)/2)),($P_f$ + $P_t$)/2);
  EXPECT_EQ(mark(mark($P_t$ + 1)),$P_t$ + 1 );
  EXPECT_EQ(mark(mark($P_t$ + 1)), $P_t$ + 1);
}

TEST(Marking, Atoms) { 
  EXPECT_GT(mark($A_f$), $A_t$);   
  EXPECT_LT(mark($A_t$),0);
  EXPECT_GT(mark(($A_f$ + $A_t$)/2), $P_t$);   
  EXPECT_GT(mark($A_t$ - 1), $P_t$);   
  EXPECT_GT(mark($A_f$ - 1), $P_t$);   
  EXPECT_EQ(mark(mark($A_f$)), $A_f$);
  EXPECT_EQ(mark(mark($A_f$ + 1)),$A_f$ + 1 );
  EXPECT_EQ(mark(mark($A_t$ + 1)), $A_t$ + 1);
  EXPECT_EQ(mark(mark($A_t$)), $A_t$);
  EXPECT_EQ(mark(mark(($A_f$ + $A_t$)/2)),($A_f$ + $A_t$)/2);
}

TEST(Marking, Bounds) { 
  EXPECT_LT(mark($P_f$), $P_f$);   
  EXPECT_LT(mark($P_t$), $P_t$);   
  EXPECT_LT(mark(($P_f$ + $P_t$)/2), $A_f$);
  EXPECT_LT(mark($P_f$-1), $X_f$);
  EXPECT_LT(mark($P_f$+1), $X_f$);   
  EXPECT_EQ(mark(mark($P_f$)), $P_f$);
  EXPECT_EQ(mark(mark($P_t$)), $P_t$);
  EXPECT_EQ(mark(mark(($P_f$ + $P_t$)/2)),($P_f$ + $P_t$)/2);
  EXPECT_EQ(mark(mark($P_f$ - 1)),$P_f$ - 1 );
  EXPECT_EQ(mark(mark($P_f$ + 1)), $P_f$ + 1);
  EXPECT_EQ(mark(mark($P_t$ - 1)),$P_t$ - 1 );
  EXPECT_EQ(mark(mark($P_t$ + 1)), $P_t$ + 1);
}

TEST(Marking, Atoms1) { 
  EXPECT_GT(mark($A_f$), $X_t$);   
  EXPECT_LT(mark($A_t$), $A_f$);
  EXPECT_GT(mark(($A_f$ + $A_t$)/2), $X_t$);
  EXPECT_GT(mark($A_t$-1), $X_t$);   
  EXPECT_GT(mark($A_f$+1), $X_t$);   
  EXPECT_EQ(mark(mark($A_f$)),$A_f$);
  EXPECT_EQ(mark(mark($A_t$)),$A_t$);
  EXPECT_EQ(mark(mark(($A_f$ + $A_t$)/2)),($A_f$ + $A_t$)/2);
  EXPECT_EQ(mark(mark($A_f$ - 1)), $A_f$ - 1 );
  EXPECT_EQ(mark(mark($A_t$ - 1)), $A_t$ - 1);
  EXPECT_EQ(mark(mark($A_f$ + 1)), $A_f$ + 1 );
  EXPECT_EQ(mark(mark($A_t$ + 1)), $A_t$ + 1);
}
