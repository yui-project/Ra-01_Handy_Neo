#include <GNSS.h>
#include <RTC.h>

static SpGnss Gnss;
bool rtcInitialized = false;
unsigned long lastProgressUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("GNSS RTC 同期プログラムを開始します。");

  // RTC の初期化
  RTC.begin();

  // GNSS の初期化（RTC が未初期化の場合のみ）
  if (!isRtcInitialized()) {
    if (Gnss.begin() != 0) {
      Serial.println("Gnss 初期化エラー");
      while (1);
    }

    Gnss.select(GPS);
    Gnss.select(QZ_L1CA);
    Gnss.select(QZ_L1S);

    if (Gnss.start(COLD_START) != 0) {
      Serial.println("Gnss 開始エラー");
      while (1);
    }
    Serial.println("GNSS 信号を取得中です。これには数分かかる場合があります。");
  }
}

void loop() {
  if (!rtcInitialized) {
    if (syncRtcWithGnss()) {
      rtcInitialized = true;
      // GNSS の使用を停止
      Gnss.stop();
      Gnss.end();
      Serial.println("GNSS モジュールを停止しました。");
    } else {
      // 進捗表示
      unsigned long currentTime = millis();
      if (currentTime - lastProgressUpdate >= 5000) { // 5秒ごとに進捗を表示
        Serial.print(".");
      lastProgressUpdate = currentTime;
      }
    }
  } else {
    // RTC の時刻を表示
    printCurrentTime();
    delay(10000); // 1秒ごとに時刻を表示
  }
}

bool syncRtcWithGnss() {
  if (Gnss.waitUpdate(1000)) { // 1000ミリ秒待機
    SpNavData navData;
    Gnss.getNavData(&navData);

    if (navData.time.year >= 2000) {
      Serial.println("\nGNSS から有効な時刻を取得しました。");

      RtcTime rtc_time(
        navData.time.year,
        navData.time.month,
        navData.time.day,
        navData.time.hour,
        navData.time.minute,
        navData.time.sec
      );

      RTC.setTime(rtc_time);
      Serial.println("RTC の時刻を更新しました。");
      printCurrentTime();
      return true;
    }
  }
  return false;
}

void printCurrentTime() {
  RtcTime now = RTC.getTime();
  char buf[64];
  snprintf(buf, sizeof(buf), "現在時刻 (RTC): %04d/%02d/%02d %02d:%02d:%02d UTC",
    now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second());
  Serial.println(buf);
}

bool isRtcInitialized() {
  RtcTime now = RTC.getTime();
  return (now.year() >= 2000); // 2000年以降の日付があれば初期化済みとみなす
}

// Memo: 計測までの時間
// 本部ベランダ: 1min ~ 1min30sec
// 本部室内(地上局テーブル上): 5min程度
