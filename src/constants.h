#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// The expected accelerometer data sample frequency
constexpr int kTargetHz = 1000;
constexpr int kPredictHz = 10;

// 配列長
const int kNeedsTimeSec = 1;                    // 入力配列の時間
const int r_length = kTargetHz * kNeedsTimeSec; // 入力配列の長さ

// フィルタ
constexpr int kWindowWidth = kTargetHz;

#endif // CONSTANTS_H_
