#pragma once

#include <wdt/ErrorCodes.h>
#include <wdt/AbortChecker.h>
#include <wdt/Protocol.h>
#include <wdt/WdtOptions.h>
#include <wdt/util/EncryptionUtils.h>
#include <memory>

namespace facebook {
namespace wdt {

/// base socket class
class WdtSocket {
 public:
  WdtSocket(const int port, IAbortChecker const *abortChecker,
            const EncryptionParams &encryptionParams);

  // making the object non-copyable and non-movable
  WdtSocket(const WdtSocket &stats) = delete;
  WdtSocket &operator=(const WdtSocket &stats) = delete;
  WdtSocket(WdtSocket &&stats) = delete;
  WdtSocket &operator=(WdtSocket &&stats) = delete;

  /// tries to read nbyte data and periodically checks for abort
  int read(char *buf, int nbyte, bool tryFull = true);

  /// tries to read nbyte data with a specific and periodically checks for abort
  int readWithTimeout(char *buf, int nbyte, int timeoutMs, bool tryFull = true);

  /// tries to write nbyte data and periodically checks for abort, if retry is
  /// true, socket tries to write as long as it makes some progress within a
  /// write timeout
  int write(char *buf, int nbyte, bool retry = false);

  /// closes current connection
  virtual void closeConnection();

  /// @return     current fd
  int getFd() const;

  /// @return     port
  int getPort() const;

  /// @return     current encryption type
  EncryptionType getEncryptionType() const;

  /// @return     possible non-retryable error
  ErrorCode getNonRetryableErrCode() const;

  /// @return     read error code
  ErrorCode getReadErrCode() const;

  /// @return     write error code
  ErrorCode getWriteErrCode() const;

  virtual ~WdtSocket();

 protected:
  int readInternal(char *buf, int nbyte, int timeoutMs, bool tryFull);

  int writeInternal(char *buf, int nbyte, int timeoutMs, bool retry);

  void readEncryptionSettingsOnce(int timeoutMs);

  void writeEncryptionSettingsOnce();

  int port_{-1};
  IAbortChecker const *abortChecker_;
  int fd_{-1};

  EncryptionParams encryptionParams_;
  bool encryptionSettingsWritten_{false};
  bool encryptionSettingsRead_{false};
  AESEncryptor encryptor_;
  AESDecryptor decryptor_;
  /// buffer used to encrypt/decrypt
  char buf_[Protocol::kMaxEncryption];

  /// If this is true, then if we get a cmd other than encryption cmd from the
  /// peer, we expect the other side to not be encryption aware. We turn off
  /// encryption in that case
  bool supportUnencryptedPeer_{false};

  // need two error codes because a socket does bi-directional communication
  ErrorCode readErrorCode_{OK};
  ErrorCode writeErrorCode_{OK};

  /// options
  const WdtOptions &options_;
};
}
}
