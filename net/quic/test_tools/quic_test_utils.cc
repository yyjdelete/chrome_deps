// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/test_tools/quic_test_utils.h"

#include "base/stl_util.h"
#include "net/quic/crypto/crypto_framer.h"
#include "net/quic/crypto/crypto_utils.h"

using std::max;
using std::min;
using std::string;
using testing::_;

namespace net {
namespace test {

MockFramerVisitor::MockFramerVisitor() {
  // By default, we want to accept packets.
  ON_CALL(*this, OnPacketHeader(_))
      .WillByDefault(testing::Return(true));
}

MockFramerVisitor::~MockFramerVisitor() {
}

bool NoOpFramerVisitor::OnPacketHeader(const QuicPacketHeader& header) {
  return true;
}

FramerVisitorCapturingAcks::FramerVisitorCapturingAcks() {
}

FramerVisitorCapturingAcks::~FramerVisitorCapturingAcks() {
}

bool FramerVisitorCapturingAcks::OnPacketHeader(
    const QuicPacketHeader& header) {
  header_ = header;
  return true;
}

void FramerVisitorCapturingAcks::OnAckFrame(const QuicAckFrame& frame) {
  ack_.reset(new QuicAckFrame(frame));
}

void FramerVisitorCapturingAcks::OnCongestionFeedbackFrame(
    const QuicCongestionFeedbackFrame& frame) {
  feedback_.reset(new QuicCongestionFeedbackFrame(frame));
}

FramerVisitorCapturingPublicReset::FramerVisitorCapturingPublicReset() {
}

FramerVisitorCapturingPublicReset::~FramerVisitorCapturingPublicReset() {
}

void FramerVisitorCapturingPublicReset::OnPublicResetPacket(
    const QuicPublicResetPacket& public_reset) {
  public_reset_packet_ = public_reset;
}

MockConnectionVisitor::MockConnectionVisitor() {
}

MockConnectionVisitor::~MockConnectionVisitor() {
}

MockHelper::MockHelper() {
}

MockHelper::~MockHelper() {
}

const QuicClock* MockHelper::GetClock() const {
  return &clock_;
}

QuicRandom* MockHelper::GetRandomGenerator() {
  return &random_generator_;
}

MockConnection::MockConnection(QuicGuid guid, IPEndPoint address)
    : QuicConnection(guid, address, new MockHelper()),
      helper_(helper()) {
}

MockConnection::MockConnection(QuicGuid guid,
                               IPEndPoint address,
                               QuicConnectionHelperInterface* helper)
    : QuicConnection(guid, address, helper),
      helper_(helper) {
}

MockConnection::~MockConnection() {
}

PacketSavingConnection::PacketSavingConnection(QuicGuid guid,
                                               IPEndPoint address)
    : MockConnection(guid, address) {
}

PacketSavingConnection::~PacketSavingConnection() {
  STLDeleteElements(&packets_);
}

bool PacketSavingConnection::SendOrQueuePacket(
    QuicPacketSequenceNumber sequence_number,
    QuicPacket* packet,
    bool force) {
  packets_.push_back(packet);
  return true;
}

MockSession::MockSession(QuicConnection* connection, bool is_server)
    : QuicSession(connection, is_server) {
  ON_CALL(*this, WriteData(_, _, _, _))
      .WillByDefault(testing::Return(QuicConsumedData(0, false)));
}

MockSession::~MockSession() {
}

MockSendAlgorithm::MockSendAlgorithm() {
}

MockSendAlgorithm::~MockSendAlgorithm() {
}

namespace {

string HexDumpWithMarks(const char* data, int length,
                        const bool* marks, int mark_length) {
  static const char kHexChars[] = "0123456789abcdef";
  static const int kColumns = 4;

  const int kSizeLimit = 1024;
  if (length > kSizeLimit || mark_length > kSizeLimit) {
    LOG(ERROR) << "Only dumping first " << kSizeLimit << " bytes.";
    length = min(length, kSizeLimit);
    mark_length = min(mark_length, kSizeLimit);
  }

  string hex;
  for (const char* row = data; length > 0;
       row += kColumns, length -= kColumns) {
    for (const char *p = row; p < row + 4; ++p) {
      if (p < row + length) {
        const bool mark =
            (marks && (p - data) < mark_length && marks[p - data]);
        hex += mark ? '*' : ' ';
        hex += kHexChars[(*p & 0xf0) >> 4];
        hex += kHexChars[*p & 0x0f];
        hex += mark ? '*' : ' ';
      } else {
        hex += "    ";
      }
    }
    hex = hex + "  ";

    for (const char *p = row; p < row + 4 && p < row + length; ++p)
      hex += (*p >= 0x20 && *p <= 0x7f) ? (*p) : '.';

    hex = hex + '\n';
  }
  return hex;
}

}  // namespace

void CompareCharArraysWithHexError(
    const string& description,
    const char* actual,
    const int actual_len,
    const char* expected,
    const int expected_len) {
  const int min_len = min(actual_len, expected_len);
  const int max_len = max(actual_len, expected_len);
  scoped_array<bool> marks(new bool[max_len]);
  bool identical = (actual_len == expected_len);
  for (int i = 0; i < min_len; ++i) {
    if (actual[i] != expected[i]) {
      marks[i] = true;
      identical = false;
    } else {
      marks[i] = false;
    }
  }
  for (int i = min_len; i < max_len; ++i) {
    marks[i] = true;
  }
  if (identical) return;
  ADD_FAILURE()
      << "Description:\n"
      << description
      << "\n\nExpected:\n"
      << HexDumpWithMarks(expected, expected_len, marks.get(), max_len)
      << "\nActual:\n"
      << HexDumpWithMarks(actual, actual_len, marks.get(), max_len);
}

void CompareQuicDataWithHexError(
    const string& description,
    QuicData* actual,
    QuicData* expected) {
  CompareCharArraysWithHexError(
      description,
      actual->data(), actual->length(),
      expected->data(), expected->length());
}

static QuicPacket* ConstructPacketFromHandshakeMessage(
    QuicGuid guid,
    const CryptoHandshakeMessage& message) {
  CryptoFramer crypto_framer;
  scoped_ptr<QuicData> data(crypto_framer.ConstructHandshakeMessage(message));
  QuicFramer quic_framer(QuicDecrypter::Create(kNULL),
                         QuicEncrypter::Create(kNULL));

  QuicPacketHeader header;
  header.public_header.guid = guid;
  header.public_header.flags = PACKET_PUBLIC_FLAGS_NONE;
  header.packet_sequence_number = 1;
  header.private_flags = PACKET_PRIVATE_FLAGS_NONE;
  header.fec_group = 0;

  QuicStreamFrame stream_frame(kCryptoStreamId, false, 0,
                               data->AsStringPiece());

  QuicFrame frame(&stream_frame);
  QuicFrames frames;
  frames.push_back(frame);
  return quic_framer.ConstructFrameDataPacket(header, frames);
}

QuicPacket* ConstructHandshakePacket(QuicGuid guid, CryptoTag tag) {
  CryptoHandshakeMessage message;
  message.tag = tag;
  return ConstructPacketFromHandshakeMessage(guid, message);
}

QuicPacket* ConstructClientHelloPacket(QuicGuid guid,
                                       const QuicClock* clock,
                                       QuicRandom* random_generator,
                                       const string& server_hostname) {
  QuicCryptoConfig config;
  config.SetClientDefaults();
  string nonce;
  CryptoUtils::GenerateNonce(clock, random_generator, &nonce);

  CryptoHandshakeMessage message;
  CryptoUtils::FillClientHelloMessage(config, nonce, server_hostname,
                                      &message);
  return ConstructPacketFromHandshakeMessage(guid, message);
}

QuicPacket* ConstructServerHelloPacket(QuicGuid guid,
                                       const QuicClock* clock,
                                       QuicRandom* random_generator) {
  QuicCryptoNegotiatedParams negotiated_params;
  negotiated_params.SetDefaults();
  string nonce;
  CryptoUtils::GenerateNonce(clock, random_generator, &nonce);

  CryptoHandshakeMessage message;
  CryptoUtils::FillServerHelloMessage(negotiated_params, nonce, &message);
  return ConstructPacketFromHandshakeMessage(guid, message);
}

}  // namespace test
}  // namespace net
