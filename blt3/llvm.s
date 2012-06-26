#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned int v;
} W;

typedef struct {
  int y;
  char* z;
  W w;
} X;

int factorial(int X) {
  if (X == 0) return 1;
  return X*factorial(X-1);
}

int func(X val, X* ptr) {
   printf("%d vs %d (v=%u)\n", val.y, ptr->y, val.w.v);
   if( val.w.v < ptr->w.v ) {
       printf("too low\n");
   }
}

int main(int argc, char **argv) {
  printf("%d\n", factorial(atoi(argv[1])));
  X x;
  X y;
  int z;
  func(x, &y);
}






; ModuleID = '/tmp/webcompile/_26277_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.X = type { i32, i8*, %struct.W }
%struct.W = type { i32 }

@.str = private unnamed_addr constant [17 x i8] c"%d vs %d (v=%u)\0A\00", align 1
@.str2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@str = internal constant [8 x i8] c"too low\00"

define i32 @factorial(i32 %X) nounwind uwtable readnone {
  %1 = icmp eq i32 %X, 0
  br i1 %1, label %tailrecurse._crit_edge, label %tailrecurse

tailrecurse:                                      ; preds = %tailrecurse, %0
  %X.tr2 = phi i32 [ %2, %tailrecurse ], [ %X, %0 ]
  %accumulator.tr1 = phi i32 [ %3, %tailrecurse ], [ 1, %0 ]
  %2 = add nsw i32 %X.tr2, -1
  %3 = mul nsw i32 %X.tr2, %accumulator.tr1
  %4 = icmp eq i32 %2, 0
  br i1 %4, label %tailrecurse._crit_edge, label %tailrecurse

tailrecurse._crit_edge:                           ; preds = %tailrecurse, %0
  %accumulator.tr.lcssa = phi i32 [ 1, %0 ], [ %3, %tailrecurse ]
  ret i32 %accumulator.tr.lcssa
}

define i32 @func(%struct.X* nocapture byval align 8 %val, %struct.X* nocapture %ptr) nounwind uwtable {
  %1 = getelementptr inbounds %struct.X* %val, i64 0, i32 0
  %2 = load i32* %1, align 8, !tbaa !0
  %3 = getelementptr inbounds %struct.X* %ptr, i64 0, i32 0
  %4 = load i32* %3, align 4, !tbaa !0
  %5 = getelementptr inbounds %struct.X* %val, i64 0, i32 2, i32 0
  %6 = load i32* %5, align 8, !tbaa !0
  %7 = tail call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([17 x i8]* @.str, i64 0, i64 0), i32 %2, i32 %4, i32 %6) nounwind
  %8 = getelementptr inbounds %struct.X* %ptr, i64 0, i32 2, i32 0
  %9 = load i32* %8, align 4, !tbaa !0
  %10 = icmp ult i32 %6, %9
  br i1 %10, label %11, label %12

; <label>:11                                      ; preds = %0
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([8 x i8]* @str, i64 0, i64 0))
  br label %12

; <label>:12                                      ; preds = %11, %0
  ret i32 undef
}

declare i32 @printf(i8* nocapture, ...) nounwind

define i32 @main(i32 %argc, i8** nocapture %argv) nounwind uwtable {
  %1 = getelementptr inbounds i8** %argv, i64 1
  %2 = load i8** %1, align 8, !tbaa !3
  %3 = tail call i64 @strtol(i8* nocapture %2, i8** null, i32 10) nounwind
  %4 = trunc i64 %3 to i32
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %func.exit, label %tailrecurse.i

tailrecurse.i:                                    ; preds = %tailrecurse.i, %0
  %X.tr2.i = phi i32 [ %6, %tailrecurse.i ], [ %4, %0 ]
  %accumulator.tr1.i = phi i32 [ %7, %tailrecurse.i ], [ 1, %0 ]
  %6 = add nsw i32 %X.tr2.i, -1
  %7 = mul nsw i32 %accumulator.tr1.i, %X.tr2.i
  %8 = icmp eq i32 %6, 0
  br i1 %8, label %func.exit, label %tailrecurse.i

func.exit:                                        ; preds = %tailrecurse.i, %0
  %accumulator.tr.lcssa.i = phi i32 [ 1, %0 ], [ %7, %tailrecurse.i ]
  %9 = tail call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([4 x i8]* @.str2, i64 0, i64 0), i32 %accumulator.tr.lcssa.i) nounwind
  %10 = tail call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([17 x i8]* @.str, i64 0, i64 0), i32 undef, i32 undef, i32 undef) nounwind
  %puts.i = tail call i32 @puts(i8* getelementptr inbounds ([8 x i8]* @str, i64 0, i64 0)) nounwind
  ret i32 0
}

declare i64 @strtol(i8*, i8** nocapture, i32) nounwind

declare i32 @puts(i8* nocapture) nounwind

!0 = metadata !{metadata !"int", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA", null}
!3 = metadata !{metadata !"any pointer", metadata !1}
