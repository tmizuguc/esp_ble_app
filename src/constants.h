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

// 機械学習モデルを使用するかどうか
// True: 機械学習モデルを使用した判定を行う
// False: 閾値を使用した判定を行う
const boolean UseML = false;

#endif // CONSTANTS_H_
