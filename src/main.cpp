#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <random>
#include <limits>
#include <memory>
#include <map>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <cerrno>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace pypp {

enum class TokenKind {
  Eof,
  Newline,
  Identifier,
  Number,
  String,
  Let,
  Import,
  As,
  If,
  While,
  End,
  LParen,
  RParen,
  Comma,
  Dot,
  LBrace,
  RBrace,
  Colon,
  Assign,
  Eq,
  Ne,
  Lt,
  Le,
  Gt,
  Ge,
  Plus,
  Minus,
  Star,
  Slash
};

struct Token {
  TokenKind kind;
  std::string lexeme;
  int line;
  int column;
};

class Lexer {
 public:
  explicit Lexer(std::string source) : source_(std::move(source)) {}

  std::vector<Token> Tokenize() {
    std::vector<Token> tokens;
    while (!AtEnd()) {
      char ch = Peek();
      if (ch == ' ' || ch == '\t' || ch == '\r') {
        Advance();
        continue;
      }
      if (ch == '\n') {
        tokens.push_back(MakeToken(TokenKind::Newline, "\\n"));
        Advance();
        line_ += 1;
        column_ = 1;
        continue;
      }
      if (ch == '#') {
        while (!AtEnd() && Peek() != '\n') {
          Advance();
        }
        continue;
      }
      if (std::isalpha(static_cast<unsigned char>(ch)) || ch == '_') {
        tokens.push_back(ReadIdentifier());
        continue;
      }
      if (std::isdigit(static_cast<unsigned char>(ch))) {
        tokens.push_back(ReadNumber());
        continue;
      }
      if (ch == '"') {
        tokens.push_back(ReadString());
        continue;
      }

      switch (ch) {
        case ':':
          tokens.push_back(MakeAndAdvance(TokenKind::Colon, ":"));
          break;
        case '(':
          tokens.push_back(MakeAndAdvance(TokenKind::LParen, "("));
          break;
        case ')':
          tokens.push_back(MakeAndAdvance(TokenKind::RParen, ")"));
          break;
        case ',':
          tokens.push_back(MakeAndAdvance(TokenKind::Comma, ","));
          break;
        case '.':
          tokens.push_back(MakeAndAdvance(TokenKind::Dot, "."));
          break;
        case '{':
          tokens.push_back(MakeAndAdvance(TokenKind::LBrace, "{"));
          break;
        case '}':
          tokens.push_back(MakeAndAdvance(TokenKind::RBrace, "}"));
          break;
        case '=':
          if (!AtEndAhead(1) && source_[index_ + 1] == '=') {
            tokens.push_back(MakeToken(TokenKind::Eq, "=="));
            Advance();
            Advance();
          } else {
            tokens.push_back(MakeAndAdvance(TokenKind::Assign, "="));
          }
          break;
        case '!':
          if (!AtEndAhead(1) && source_[index_ + 1] == '=') {
            tokens.push_back(MakeToken(TokenKind::Ne, "!="));
            Advance();
            Advance();
          } else {
            throw std::runtime_error("Unexpected character '!' at " + Pos());
          }
          break;
        case '<':
          if (!AtEndAhead(1) && source_[index_ + 1] == '=') {
            tokens.push_back(MakeToken(TokenKind::Le, "<="));
            Advance();
            Advance();
          } else {
            tokens.push_back(MakeAndAdvance(TokenKind::Lt, "<"));
          }
          break;
        case '>':
          if (!AtEndAhead(1) && source_[index_ + 1] == '=') {
            tokens.push_back(MakeToken(TokenKind::Ge, ">="));
            Advance();
            Advance();
          } else {
            tokens.push_back(MakeAndAdvance(TokenKind::Gt, ">"));
          }
          break;
        case '+':
          tokens.push_back(MakeAndAdvance(TokenKind::Plus, "+"));
          break;
        case '-':
          tokens.push_back(MakeAndAdvance(TokenKind::Minus, "-"));
          break;
        case '*':
          tokens.push_back(MakeAndAdvance(TokenKind::Star, "*"));
          break;
        case '/':
          tokens.push_back(MakeAndAdvance(TokenKind::Slash, "/"));
          break;
        default:
          throw std::runtime_error("Unexpected character '" + std::string(1, ch) +
                                   "' at " + Pos());
      }
    }
    tokens.push_back(Token{TokenKind::Eof, "", line_, column_});
    return tokens;
  }

 private:
  bool AtEnd() const { return index_ >= source_.size(); }
  bool AtEndAhead(std::size_t n) const { return index_ + n >= source_.size(); }

  char Peek() const { return source_[index_]; }

  char Advance() {
    char ch = source_[index_++];
    column_ += 1;
    return ch;
  }

  std::string Pos() const {
    return std::to_string(line_) + ":" + std::to_string(column_);
  }

  Token MakeToken(TokenKind kind, const std::string& lexeme) const {
    return Token{kind, lexeme, line_, column_};
  }

  Token MakeAndAdvance(TokenKind kind, const std::string& lexeme) {
    Token tok{kind, lexeme, line_, column_};
    Advance();
    return tok;
  }

  Token ReadIdentifier() {
    int start_line = line_;
    int start_col = column_;
    std::size_t start = index_;
    while (!AtEnd() &&
           (std::isalnum(static_cast<unsigned char>(Peek())) || Peek() == '_')) {
      Advance();
    }
    std::string text = source_.substr(start, index_ - start);
    TokenKind kind = TokenKind::Identifier;
    if (text == "let") {
      kind = TokenKind::Let;
    } else if (text == "import") {
      kind = TokenKind::Import;
    } else if (text == "as") {
      kind = TokenKind::As;
    } else if (text == "if") {
      kind = TokenKind::If;
    } else if (text == "while") {
      kind = TokenKind::While;
    } else if (text == "end") {
      kind = TokenKind::End;
    }
    return Token{kind, text, start_line, start_col};
  }

  Token ReadNumber() {
    int start_line = line_;
    int start_col = column_;
    std::size_t start = index_;
    while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) {
      Advance();
    }
    return Token{TokenKind::Number, source_.substr(start, index_ - start), start_line,
                 start_col};
  }

  Token ReadString() {
    int start_line = line_;
    int start_col = column_;
    Advance();  // opening quote
    std::string text;
    while (!AtEnd() && Peek() != '"') {
      char ch = Advance();
      if (ch == '\\') {
        if (AtEnd()) {
          throw std::runtime_error("Unterminated escape at " + Pos());
        }
        char esc = Advance();
        switch (esc) {
          case 'n':
            text.push_back('\n');
            break;
          case 't':
            text.push_back('\t');
            break;
          case '"':
            text.push_back('"');
            break;
          case '\\':
            text.push_back('\\');
            break;
          default:
            throw std::runtime_error("Unknown escape sequence at " + Pos());
        }
      } else {
        text.push_back(ch);
      }
    }
    if (AtEnd()) {
      throw std::runtime_error("Unterminated string at " +
                               std::to_string(start_line) + ":" +
                               std::to_string(start_col));
    }
    Advance();  // closing quote
    return Token{TokenKind::String, text, start_line, start_col};
  }

  std::string source_;
  std::size_t index_ = 0;
  int line_ = 1;
  int column_ = 1;
};

struct Instruction {
  std::string op;
  std::vector<std::string> args;
};

class Parser {
 public:
  explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

  std::vector<Instruction> ParseProgram() {
    std::vector<Instruction> out;
    SkipNewlines();
    while (!Check(TokenKind::Eof)) {
      ParseStatement(out);
      SkipNewlines();
    }
    out.push_back(Instruction{"HALT", {}});
    return out;
  }

 private:
  void ParseStatement(std::vector<Instruction>& out) {
    if (Match(TokenKind::Let)) {
      ParseLet(out);
      return;
    }
    if (Match(TokenKind::Import)) {
      ParseImport(out);
      return;
    }
    if (Match(TokenKind::If)) {
      ParseIf(out);
      return;
    }
    if (Match(TokenKind::While)) {
      ParseWhile(out);
      return;
    }
    ParseExpression(out);
    out.push_back(Instruction{"POP", {}});
  }

  void ParseLet(std::vector<Instruction>& out) {
    Token name = Consume(TokenKind::Identifier, "Expected variable name after let");
    Consume(TokenKind::Assign, "Expected '=' after variable name");
    ParseExpression(out);
    out.push_back(Instruction{"STORE", {name.lexeme}});
  }

  void ParseImport(std::vector<Instruction>& out) {
    Token first = Consume(TokenKind::Identifier, "Expected module name after import");
    std::string module = first.lexeme;
    while (Match(TokenKind::Dot)) {
      Token part = Consume(TokenKind::Identifier, "Expected identifier after '.'");
      module += "." + part.lexeme;
    }
    Consume(TokenKind::As, "Expected 'as' in import statement");
    Token alias = Consume(TokenKind::Identifier, "Expected alias after 'as'");
    out.push_back(Instruction{"IMPORT", {module, alias.lexeme}});
  }

  void ParseIf(std::vector<Instruction>& out) {
    ParseExpression(out);
    Consume(TokenKind::Colon, "Expected ':' after if condition");
    RequireStatementBreak("Expected newline after if header");
    int jump_if_false_index = static_cast<int>(out.size());
    out.push_back(Instruction{"JZ", {"-1"}});
    ParseBlockUntilEnd(out);
    out[static_cast<std::size_t>(jump_if_false_index)].args[0] =
        std::to_string(static_cast<int>(out.size()));
  }

  void ParseWhile(std::vector<Instruction>& out) {
    int loop_start = static_cast<int>(out.size());
    ParseExpression(out);
    Consume(TokenKind::Colon, "Expected ':' after while condition");
    RequireStatementBreak("Expected newline after while header");
    int jump_if_false_index = static_cast<int>(out.size());
    out.push_back(Instruction{"JZ", {"-1"}});
    ParseBlockUntilEnd(out);
    out.push_back(Instruction{"JMP", {std::to_string(loop_start)}});
    out[static_cast<std::size_t>(jump_if_false_index)].args[0] =
        std::to_string(static_cast<int>(out.size()));
  }

  void ParseBlockUntilEnd(std::vector<Instruction>& out) {
    SkipNewlines();
    while (!Check(TokenKind::End) && !Check(TokenKind::Eof)) {
      ParseStatement(out);
      RequireStatementBreak("Expected newline between statements");
      SkipNewlines();
    }
    Consume(TokenKind::End, "Expected 'end' to close block");
  }

  void ParseExpression(std::vector<Instruction>& out) { ParseComparison(out); }

  void ParseComparison(std::vector<Instruction>& out) {
    ParseTerm(out);
    while (true) {
      if (Match(TokenKind::Eq)) {
        ParseTerm(out);
        out.push_back(Instruction{"CMP_EQ", {}});
      } else if (Match(TokenKind::Ne)) {
        ParseTerm(out);
        out.push_back(Instruction{"CMP_NE", {}});
      } else if (Match(TokenKind::Lt)) {
        ParseTerm(out);
        out.push_back(Instruction{"CMP_LT", {}});
      } else if (Match(TokenKind::Le)) {
        ParseTerm(out);
        out.push_back(Instruction{"CMP_LE", {}});
      } else if (Match(TokenKind::Gt)) {
        ParseTerm(out);
        out.push_back(Instruction{"CMP_GT", {}});
      } else if (Match(TokenKind::Ge)) {
        ParseTerm(out);
        out.push_back(Instruction{"CMP_GE", {}});
      } else {
        break;
      }
    }
  }

  void ParseTerm(std::vector<Instruction>& out) {
    ParseFactor(out);
    while (true) {
      if (Match(TokenKind::Plus)) {
        ParseFactor(out);
        out.push_back(Instruction{"ADD", {}});
      } else if (Match(TokenKind::Minus)) {
        ParseFactor(out);
        out.push_back(Instruction{"SUB", {}});
      } else {
        break;
      }
    }
  }

  void ParseFactor(std::vector<Instruction>& out) {
    ParseUnary(out);
    while (true) {
      if (Match(TokenKind::Star)) {
        ParseUnary(out);
        out.push_back(Instruction{"MUL", {}});
      } else if (Match(TokenKind::Slash)) {
        ParseUnary(out);
        out.push_back(Instruction{"DIV", {}});
      } else {
        break;
      }
    }
  }

  void ParseUnary(std::vector<Instruction>& out) {
    if (Match(TokenKind::Minus)) {
      ParseUnary(out);
      out.push_back(Instruction{"NEG", {}});
      return;
    }
    ParsePrimary(out);
  }

  void ParsePrimary(std::vector<Instruction>& out) {
    if (Match(TokenKind::Number)) {
      out.push_back(Instruction{"PUSH_INT", {Previous().lexeme}});
      return;
    }
    if (Match(TokenKind::String)) {
      out.push_back(Instruction{"PUSH_STR", {Previous().lexeme}});
      return;
    }
    if (Match(TokenKind::Identifier)) {
      std::string base = Previous().lexeme;
      std::vector<std::string> parts;
      while (Match(TokenKind::Dot)) {
        Token part = Consume(TokenKind::Identifier, "Expected identifier after '.'");
        parts.push_back(part.lexeme);
      }

      if (Match(TokenKind::LParen)) {
        std::string path = base;
        for (const std::string& p : parts) {
          path += "." + p;
        }
        int argc = 0;
        if (!Check(TokenKind::RParen)) {
          while (true) {
            ParseExpression(out);
            argc += 1;
            if (!Match(TokenKind::Comma)) {
              break;
            }
          }
        }
        Consume(TokenKind::RParen, "Expected ')' after call arguments");
        out.push_back(Instruction{"CALL", {path, std::to_string(argc)}});
      } else {
        out.push_back(Instruction{"LOAD", {base}});
        for (const std::string& p : parts) {
          out.push_back(Instruction{"GET_FIELD", {p}});
        }
      }
      return;
    }
    if (Match(TokenKind::LBrace)) {
      out.push_back(Instruction{"NEW_OBJ", {}});
      SkipNewlines();
      if (!Check(TokenKind::RBrace)) {
        while (true) {
          std::string key;
          if (Match(TokenKind::Identifier) || Match(TokenKind::String)) {
            key = Previous().lexeme;
          } else {
            throw std::runtime_error("Expected object key at " + CurrentPos());
          }
          Consume(TokenKind::Colon, "Expected ':' after object key");
          ParseExpression(out);
          out.push_back(Instruction{"SET_FIELD", {key}});
          SkipNewlines();
          if (!Match(TokenKind::Comma)) {
            break;
          }
          SkipNewlines();
        }
      }
      Consume(TokenKind::RBrace, "Expected '}' after object literal");
      return;
    }
    if (Match(TokenKind::LParen)) {
      ParseExpression(out);
      Consume(TokenKind::RParen, "Expected ')' after expression");
      return;
    }
    throw std::runtime_error("Unexpected token at " + CurrentPos());
  }

  void SkipNewlines() {
    while (Match(TokenKind::Newline)) {
    }
  }

  void RequireStatementBreak(const std::string& message) {
    if (!Match(TokenKind::Newline) && !Check(TokenKind::Eof) &&
        !Check(TokenKind::End)) {
      throw std::runtime_error(message + " at " + CurrentPos());
    }
  }

  bool Match(TokenKind kind) {
    if (Check(kind)) {
      index_ += 1;
      return true;
    }
    return false;
  }

  bool Check(TokenKind kind) const { return Peek().kind == kind; }

  Token Consume(TokenKind kind, const std::string& message) {
    if (Check(kind)) {
      return Advance();
    }
    throw std::runtime_error(message + " at " + CurrentPos());
  }

  Token Advance() { return tokens_[index_++]; }

  const Token& Peek() const { return tokens_[index_]; }

  const Token& Previous() const { return tokens_[index_ - 1]; }

  std::string CurrentPos() const {
    const Token& tok = Peek();
    return std::to_string(tok.line) + ":" + std::to_string(tok.column);
  }

  std::vector<Token> tokens_;
  std::size_t index_ = 0;
};

struct Object;
struct List;
using ObjectPtr = std::shared_ptr<Object>;
using ListPtr = std::shared_ptr<List>;
using Value = std::variant<int, std::string, ObjectPtr, ListPtr>;

struct Object {
  std::unordered_map<std::string, Value> fields;
};

struct List {
  std::vector<Value> items;
};

std::string ValueToString(const Value& value) {
  if (std::holds_alternative<int>(value)) {
    return std::to_string(std::get<int>(value));
  }
  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
  }
  if (std::holds_alternative<ListPtr>(value)) {
    ListPtr list = std::get<ListPtr>(value);
    if (!list) {
      return "[]";
    }
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < list->items.size(); ++i) {
      if (i > 0) {
        out << ", ";
      }
      out << ValueToString(list->items[i]);
    }
    out << "]";
    return out.str();
  }
  return "<object>";
}

int ValueAsInt(const Value& value, const std::string& context) {
  if (!std::holds_alternative<int>(value)) {
    throw std::runtime_error(context + ": expected int");
  }
  return std::get<int>(value);
}

bool ValueIsTruthy(const Value& value) {
  if (std::holds_alternative<int>(value)) {
    return std::get<int>(value) != 0;
  }
  if (std::holds_alternative<std::string>(value)) {
    return !std::get<std::string>(value).empty();
  }
  if (std::holds_alternative<ListPtr>(value)) {
    ListPtr list = std::get<ListPtr>(value);
    return list && !list->items.empty();
  }
  return std::get<ObjectPtr>(value) != nullptr;
}

struct Pixel {
  int r = 0;
  int g = 0;
  int b = 0;
};

struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
};

struct GraphicsState {
  int width = 0;
  int height = 0;
  std::vector<Pixel> pixels;
  struct SpriteTexel {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
  };
  struct SpriteAsset {
    int width = 0;
    int height = 0;
    std::vector<SpriteTexel> texels;
  };
  std::vector<SpriteAsset> sprites;
  struct AnimClip {
    int first_sprite = 0;
    int frame_count = 0;
    int frame_ticks = 1;
    int playback_mode = 1;  // 0=once, 1=loop, 2=ping-pong
  };
  std::vector<AnimClip> anims;
  struct ShaderOp {
    int mode = 0;
    int p1 = 0;
    int p2 = 0;
    int p3 = 0;
  };
  struct ShaderProgram {
    std::vector<ShaderOp> ops;
  };
  std::vector<ShaderProgram> shader_programs;
#ifdef _WIN32
  HWND hwnd = nullptr;
  bool window_open = false;
  std::array<bool, 256> key_state{};
  std::vector<std::uint32_t> rgba_buffer;
  bool keep_aspect = false;
  int aspect_w = 0;
  int aspect_h = 0;
  RECT viewport_rect{0, 0, 0, 0};
  int mouse_client_x = -1;
  int mouse_client_y = -1;
  bool mouse_left_down = false;
  bool mouse_right_down = false;
  bool mouse_middle_down = false;
  bool mouse_left_prev = false;
  bool mouse_lock = false;
  bool mouse_hidden = false;
  int mouse_dx_acc = 0;
  int mouse_dy_acc = 0;
  bool suppress_mouse_delta = false;
#endif
  int shader_mode = 0;
  int shader_p1 = 0;
  int shader_p2 = 0;
  int shader_p3 = 0;
  int shader_program_active = -1;
  int present_frame = 0;
  int refresh_rate_hz = 0;
  std::chrono::steady_clock::time_point next_frame_time{};
  bool frame_sync_ready = false;

  bool IsOpen() const { return width > 0 && height > 0; }

  void Open(int w, int h) {
    if (w <= 0 || h <= 0) {
      throw std::runtime_error("gfx.open expects positive width/height");
    }
    width = w;
    height = h;
    pixels.assign(static_cast<std::size_t>(w * h), Pixel{0, 0, 0});
    present_frame = 0;
    shader_mode = 0;
    shader_p1 = 0;
    shader_p2 = 0;
    shader_p3 = 0;
    shader_program_active = -1;
  }

  void Clear(int r, int g, int b) {
    EnsureOpen("gfx.clear");
    Pixel p{ClampColor(r), ClampColor(g), ClampColor(b)};
    std::fill(pixels.begin(), pixels.end(), p);
  }

  void PixelAt(int x, int y, int r, int g, int b) {
    EnsureOpen("gfx.pixel");
    if (x < 0 || y < 0 || x >= width || y >= height) {
      return;
    }
    pixels[static_cast<std::size_t>(y * width + x)] =
        Pixel{ClampColor(r), ClampColor(g), ClampColor(b)};
  }

  void PixelAtFast(int x, int y, int r, int g, int b) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
      return;
    }
    pixels[static_cast<std::size_t>(y * width + x)] =
        Pixel{ClampColor(r), ClampColor(g), ClampColor(b)};
  }

  void Line(int x1, int y1, int x2, int y2, int r, int g, int b) {
    EnsureOpen("gfx.line");
    Pixel color{ClampColor(r), ClampColor(g), ClampColor(b)};

    int dx = std::abs(x2 - x1);
    int dy = -std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;

    while (true) {
      SetPixelRaw(x1, y1, color);
      if (x1 == x2 && y1 == y2) {
        break;
      }
      int e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x1 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y1 += sy;
      }
    }
  }

  void Rect(int x, int y, int w, int h, int r, int g, int b) {
    EnsureOpen("gfx.rect");
    if (w <= 0 || h <= 0) {
      return;
    }
    Pixel color{ClampColor(r), ClampColor(g), ClampColor(b)};
    for (int yy = 0; yy < h; ++yy) {
      for (int xx = 0; xx < w; ++xx) {
        SetPixelRaw(x + xx, y + yy, color);
      }
    }
  }

  void RectOutline(int x, int y, int w, int h, int r, int g, int b) {
    EnsureOpen("gfx.rect_outline");
    if (w <= 0 || h <= 0) {
      return;
    }
    Pixel color{ClampColor(r), ClampColor(g), ClampColor(b)};
    for (int xx = 0; xx < w; ++xx) {
      SetPixelRaw(x + xx, y, color);
      SetPixelRaw(x + xx, y + h - 1, color);
    }
    for (int yy = 0; yy < h; ++yy) {
      SetPixelRaw(x, y + yy, color);
      SetPixelRaw(x + w - 1, y + yy, color);
    }
  }

  void Circle(int cx, int cy, int radius, int r, int g, int b) {
    EnsureOpen("gfx.circle");
    if (radius <= 0) {
      return;
    }
    Pixel color{ClampColor(r), ClampColor(g), ClampColor(b)};
    int rr = radius * radius;
    for (int y = -radius; y <= radius; ++y) {
      for (int x = -radius; x <= radius; ++x) {
        int d = x * x + y * y;
        if (d <= rr) {
          SetPixelRaw(cx + x, cy + y, color);
        }
      }
    }
  }

  void Save(const std::string& path) const {
    EnsureOpen("gfx.save");
    std::filesystem::path out(path);
    if (out.has_parent_path()) {
      std::filesystem::create_directories(out.parent_path());
    }

    std::ofstream stream(path, std::ios::binary);
    if (!stream) {
      throw std::runtime_error("Failed to write image: " + path);
    }
    stream << "P3\n" << width << " " << height << "\n255\n";
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        const Pixel& p = pixels[static_cast<std::size_t>(y * width + x)];
        stream << p.r << " " << p.g << " " << p.b << "\n";
      }
    }
  }

  void SaveFrame(const std::string& prefix, int frame_index) const {
    EnsureOpen("gfx.save_frame");
    std::ostringstream name;
    name << prefix << "_" << std::setw(4) << std::setfill('0')
         << std::max(0, frame_index) << ".ppm";
    Save(name.str());
  }

  int Width() const {
    EnsureOpen("gfx.width");
    return width;
  }

  int Height() const {
    EnsureOpen("gfx.height");
    return height;
  }

  void OpenWindow(int w, int h, const std::string& title) {
    Open(w, h);
#ifdef _WIN32
    static bool class_registered = false;
    if (!class_registered) {
      WNDCLASSW wc{};
      wc.lpfnWndProc = &GraphicsState::WndProcStatic;
      wc.hInstance = GetModuleHandleW(nullptr);
      wc.lpszClassName = L"pypp_live_window";
      wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
      RegisterClassW(&wc);
      class_registered = true;
    }

    std::wstring wtitle(title.begin(), title.end());
    hwnd = CreateWindowExW(
        0, L"pypp_live_window", wtitle.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, w + 32, h + 39, nullptr, nullptr,
        GetModuleHandleW(nullptr), this);
    if (!hwnd) {
      throw std::runtime_error("Failed to create window");
    }
    window_open = true;
    keep_aspect = false;
    aspect_w = 0;
    aspect_h = 0;
    rgba_buffer.assign(static_cast<std::size_t>(width * height), 0);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    UpdateViewportRect();
#else
    (void)title;
    throw std::runtime_error("Live windowing currently supported on Windows only");
#endif
  }

  void OpenWindowRatio(int w, int h, int ratio_w, int ratio_h,
                       const std::string& title) {
    if (ratio_w <= 0 || ratio_h <= 0) {
      throw std::runtime_error("gfx.window_ratio expects positive ratio");
    }
    OpenWindow(w, h, title);
#ifdef _WIN32
    keep_aspect = true;
    aspect_w = ratio_w;
    aspect_h = ratio_h;
    UpdateViewportRect();
#else
    (void)ratio_w;
    (void)ratio_h;
#endif
  }

  void SetKeepAspect(int enabled) {
#ifdef _WIN32
    keep_aspect = (enabled != 0);
    if (!keep_aspect) {
      aspect_w = 0;
      aspect_h = 0;
    } else if (aspect_w <= 0 || aspect_h <= 0) {
      aspect_w = width;
      aspect_h = height;
    }
    UpdateViewportRect();
#else
    (void)enabled;
#endif
  }

  void SetRefreshRate(int hz) {
    if (hz <= 0) {
      refresh_rate_hz = 0;
      frame_sync_ready = false;
      return;
    }
    if (hz > 1000) {
      hz = 1000;
    }
    refresh_rate_hz = hz;
    frame_sync_ready = false;
  }

  int PollEvents() {
#ifdef _WIN32
    if (!window_open) {
      return 0;
    }
    mouse_left_prev = mouse_left_down;
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    if (window_open && hwnd) {
      if (mouse_hidden || mouse_lock) {
        SetMouseVisibleImpl(false);
      } else {
        SetMouseVisibleImpl(true);
      }
      if (mouse_lock) {
        RECT client{};
        if (GetClientRect(hwnd, &client)) {
          POINT center{(client.right - client.left) / 2,
                       (client.bottom - client.top) / 2};
          POINT screen_center = center;
          ClientToScreen(hwnd, &screen_center);
          suppress_mouse_delta = true;
          SetCursorPos(screen_center.x, screen_center.y);
          mouse_client_x = center.x;
          mouse_client_y = center.y;
        }
      }
    }
    return window_open ? 1 : 0;
#else
    return 0;
#endif
  }

  int Present() {
#ifdef _WIN32
    if (!window_open || !hwnd) {
      return 0;
    }
    if (rgba_buffer.size() != static_cast<std::size_t>(width * height)) {
      rgba_buffer.assign(static_cast<std::size_t>(width * height), 0);
    }
    BuildPresentBuffer();

    HDC hdc = GetDC(hwnd);
    BlitFrameToHdc(hdc);
    ReleaseDC(hwnd, hdc);
    present_frame += 1;
    SyncFrame();
    return 1;
#else
    return 0;
#endif
  }

  int SyncFrame() {
    if (refresh_rate_hz <= 0) {
      return 0;
    }
    const auto frame_dt =
        std::chrono::nanoseconds(1000000000LL / refresh_rate_hz);
    const auto now = std::chrono::steady_clock::now();
    if (!frame_sync_ready) {
      next_frame_time = now + frame_dt;
      frame_sync_ready = true;
      return 1;
    }
    if (now < next_frame_time) {
      std::this_thread::sleep_for(next_frame_time - now);
    }
    auto after_sleep = std::chrono::steady_clock::now();
    while (next_frame_time <= after_sleep) {
      next_frame_time += frame_dt;
    }
    return 1;
  }

  int KeyDown(int code) const {
#ifdef _WIN32
    if (code < 0 || code >= 256) {
      return 0;
    }
    return key_state[static_cast<std::size_t>(code)] ? 1 : 0;
#else
    (void)code;
    return 0;
#endif
  }

  int IsClosed() const {
#ifdef _WIN32
    return window_open ? 0 : 1;
#else
    return 1;
#endif
  }

  void CloseWindow() {
#ifdef _WIN32
    SetMouseVisibleImpl(true);
    if (hwnd) {
      DestroyWindow(hwnd);
      hwnd = nullptr;
    }
    window_open = false;
#endif
  }

  int LoadSprite(const std::string& path) {
#ifdef _WIN32
    (void)GetGdiPlusRuntime();
    std::filesystem::path p(path);
    std::wstring wp = p.wstring();
    std::unique_ptr<Gdiplus::Bitmap> bmp(
        Gdiplus::Bitmap::FromFile(wp.c_str(), FALSE));
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
      throw std::runtime_error("gfx.load_sprite failed for: " + path);
    }
    const int w = static_cast<int>(bmp->GetWidth());
    const int h = static_cast<int>(bmp->GetHeight());
    if (w <= 0 || h <= 0) {
      throw std::runtime_error("gfx.load_sprite invalid image size: " + path);
    }

    SpriteAsset sprite;
    sprite.width = w;
    sprite.height = h;
    sprite.texels.resize(static_cast<std::size_t>(w * h));

    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        Gdiplus::Color c;
        if (bmp->GetPixel(static_cast<INT>(x), static_cast<INT>(y), &c) !=
            Gdiplus::Ok) {
          throw std::runtime_error("gfx.load_sprite pixel read failed: " + path);
        }
        SpriteTexel t;
        t.r = c.GetR();
        t.g = c.GetG();
        t.b = c.GetB();
        t.a = c.GetA();
        sprite.texels[static_cast<std::size_t>(y * w + x)] = t;
      }
    }

    sprites.push_back(std::move(sprite));
    return static_cast<int>(sprites.size() - 1);
#else
    (void)path;
    throw std::runtime_error(
        "gfx.load_sprite is currently implemented for Windows builds only");
#endif
  }

  void DrawSprite(int sprite_id, int x, int y) {
    EnsureOpen("gfx.draw_sprite");
    const SpriteAsset& s = GetSprite(sprite_id, "gfx.draw_sprite");
    BlitSprite(s, x, y, s.width, s.height);
  }

  void DrawSpriteScaled(int sprite_id, int x, int y, int w, int h) {
    EnsureOpen("gfx.draw_sprite_scaled");
    if (w <= 0 || h <= 0) {
      return;
    }
    const SpriteAsset& s = GetSprite(sprite_id, "gfx.draw_sprite_scaled");
    BlitSprite(s, x, y, w, h);
  }

  void ShaderSet(int mode, int p1, int p2, int p3) {
    shader_mode = mode;
    shader_p1 = p1;
    shader_p2 = p2;
    shader_p3 = p3;
    shader_program_active = -1;
  }

  void ShaderClear() {
    shader_mode = 0;
    shader_p1 = 0;
    shader_p2 = 0;
    shader_p3 = 0;
    shader_program_active = -1;
  }

  int ShaderCreate() {
    ShaderProgram program;
    shader_programs.push_back(std::move(program));
    return static_cast<int>(shader_programs.size() - 1);
  }

  void ShaderProgramClear(int program_id) {
    if (program_id < 0 ||
        static_cast<std::size_t>(program_id) >= shader_programs.size()) {
      throw std::runtime_error("gfx.shader_program_clear invalid program id");
    }
    shader_programs[static_cast<std::size_t>(program_id)].ops.clear();
  }

  void ShaderAdd(int program_id, int mode, int p1, int p2, int p3) {
    if (program_id < 0 ||
        static_cast<std::size_t>(program_id) >= shader_programs.size()) {
      throw std::runtime_error("gfx.shader_add invalid program id");
    }
    if (mode <= 0) {
      throw std::runtime_error("gfx.shader_add expects mode > 0");
    }
    ShaderOp op;
    op.mode = mode;
    op.p1 = p1;
    op.p2 = p2;
    op.p3 = p3;
    shader_programs[static_cast<std::size_t>(program_id)].ops.push_back(op);
  }

  int ShaderProgramLen(int program_id) const {
    if (program_id < 0 ||
        static_cast<std::size_t>(program_id) >= shader_programs.size()) {
      throw std::runtime_error("gfx.shader_program_len invalid program id");
    }
    return static_cast<int>(
        shader_programs[static_cast<std::size_t>(program_id)].ops.size());
  }

  void ShaderUseProgram(int program_id) {
    if (program_id < 0 ||
        static_cast<std::size_t>(program_id) >= shader_programs.size()) {
      throw std::runtime_error("gfx.shader_use_program invalid program id");
    }
    shader_program_active = program_id;
    shader_mode = 0;
  }

  int AnimRegister(int first_sprite, int frame_count, int frame_ticks,
                   int loop_flag) {
    EnsureOpen("gfx.anim_register");
    if (frame_count <= 0) {
      throw std::runtime_error("gfx.anim_register expects frame_count > 0");
    }
    if (frame_ticks <= 0) {
      throw std::runtime_error("gfx.anim_register expects frame_ticks > 0");
    }
    if (first_sprite < 0 ||
        static_cast<std::size_t>(first_sprite + frame_count - 1) >=
            sprites.size()) {
      throw std::runtime_error(
          "gfx.anim_register sprite range out of loaded sprite ids");
    }
    AnimClip clip;
    clip.first_sprite = first_sprite;
    clip.frame_count = frame_count;
    clip.frame_ticks = frame_ticks;
    if (loop_flag < 0 || loop_flag > 2) {
      throw std::runtime_error("gfx.anim_register mode must be 0, 1, or 2");
    }
    clip.playback_mode = loop_flag;
    anims.push_back(clip);
    return static_cast<int>(anims.size() - 1);
  }

  int AnimFrame(int anim_id, int tick) const {
    if (anim_id < 0 || static_cast<std::size_t>(anim_id) >= anims.size()) {
      throw std::runtime_error("gfx.anim_frame invalid animation id");
    }
    if (tick < 0) {
      tick = present_frame;
    }
    const AnimClip& clip = anims[static_cast<std::size_t>(anim_id)];
    int frame_idx = tick / clip.frame_ticks;
    if (clip.playback_mode == 1) {
      frame_idx = frame_idx % clip.frame_count;
    } else if (clip.playback_mode == 2 && clip.frame_count > 1) {
      const int period = (clip.frame_count * 2) - 2;
      int k = frame_idx % period;
      if (k >= clip.frame_count) {
        k = period - k;
      }
      frame_idx = k;
    } else if (frame_idx >= clip.frame_count) {
      frame_idx = clip.frame_count - 1;
    }
    return clip.first_sprite + frame_idx;
  }

  int AnimLength(int anim_id) const {
    if (anim_id < 0 || static_cast<std::size_t>(anim_id) >= anims.size()) {
      throw std::runtime_error("gfx.anim_length invalid animation id");
    }
    const AnimClip& clip = anims[static_cast<std::size_t>(anim_id)];
    return clip.frame_count;
  }

  void AnimDraw(int anim_id, int tick, int x, int y) {
    const int sprite_id = AnimFrame(anim_id, tick);
    DrawSprite(sprite_id, x, y);
  }

  void AnimDrawScaled(int anim_id, int tick, int x, int y, int w, int h) {
    const int sprite_id = AnimFrame(anim_id, tick);
    DrawSpriteScaled(sprite_id, x, y, w, h);
  }

  void Text(int x, int y, const std::string& text, int r, int g, int b) {
    EnsureOpen("gfx.text");
    Pixel color{ClampColor(r), ClampColor(g), ClampColor(b)};
    int cx = x;
    int cy = y;
    for (char raw : text) {
      if (raw == '\n') {
        cx = x;
        cy += 8;
        continue;
      }
      DrawGlyph5x7(cx, cy, raw, color);
      cx += 6;
    }
  }

  int MouseX() const {
#ifdef _WIN32
    return mouse_client_x;
#else
    return -1;
#endif
  }

  int MouseY() const {
#ifdef _WIN32
    return mouse_client_y;
#else
    return -1;
#endif
  }

  int MouseDown(int button_code) const {
#ifdef _WIN32
    if (button_code == 0) {
      return mouse_left_down ? 1 : 0;
    }
    if (button_code == 1) {
      return mouse_right_down ? 1 : 0;
    }
    if (button_code == 2) {
      return mouse_middle_down ? 1 : 0;
    }
    return 0;
#else
    (void)button_code;
    return 0;
#endif
  }

  int ConsumeMouseDX() {
#ifdef _WIN32
    int v = mouse_dx_acc;
    mouse_dx_acc = 0;
    return v;
#else
    return 0;
#endif
  }

  int ConsumeMouseDY() {
#ifdef _WIN32
    int v = mouse_dy_acc;
    mouse_dy_acc = 0;
    return v;
#else
    return 0;
#endif
  }

  void SetMouseLock(int enabled) {
#ifdef _WIN32
    mouse_lock = (enabled != 0);
    mouse_dx_acc = 0;
    mouse_dy_acc = 0;
#else
    (void)enabled;
#endif
  }

  void SetMouseVisible(int enabled) {
#ifdef _WIN32
    mouse_hidden = (enabled == 0);
    SetMouseVisibleImpl(!mouse_hidden);
#else
    (void)enabled;
#endif
  }

  int Button(int x, int y, int w, int h) {
    EnsureOpen("gfx.button");
    if (w <= 0 || h <= 0) {
      return 0;
    }
    bool hover = mouse_client_x >= x && mouse_client_x < (x + w) &&
                 mouse_client_y >= y && mouse_client_y < (y + h);
    if (hover) {
      Rect(x, y, w, h, 90, 120, 190);
      RectOutline(x, y, w, h, 220, 235, 255);
    } else {
      Rect(x, y, w, h, 55, 70, 110);
      RectOutline(x, y, w, h, 140, 165, 230);
    }
    return (hover && mouse_left_down && !mouse_left_prev) ? 1 : 0;
  }

  const SpriteAsset& GetSpriteAsset(int sprite_id, const std::string& fn) const {
    return GetSprite(sprite_id, fn);
  }

 private:
  void EnsureOpen(const std::string& fn) const {
    if (!IsOpen()) {
      throw std::runtime_error(fn + " called before gfx.open");
    }
  }

  static int ClampColor(int v) { return std::max(0, std::min(255, v)); }

  void SetPixelRaw(int x, int y, const Pixel& pixel) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
      return;
    }
    pixels[static_cast<std::size_t>(y * width + x)] = pixel;
  }

  const SpriteAsset& GetSprite(int sprite_id, const std::string& fn) const {
    if (sprite_id < 0 || static_cast<std::size_t>(sprite_id) >= sprites.size()) {
      throw std::runtime_error(fn + ": invalid sprite id " +
                               std::to_string(sprite_id));
    }
    return sprites[static_cast<std::size_t>(sprite_id)];
  }

  void BlitSprite(const SpriteAsset& s, int dst_x, int dst_y, int dst_w,
                  int dst_h) {
    for (int yy = 0; yy < dst_h; ++yy) {
      const int sy = (yy * s.height) / dst_h;
      for (int xx = 0; xx < dst_w; ++xx) {
        const int sx = (xx * s.width) / dst_w;
        const SpriteTexel& t =
            s.texels[static_cast<std::size_t>(sy * s.width + sx)];
        if (t.a == 0) {
          continue;
        }
        const int tx = dst_x + xx;
        const int ty = dst_y + yy;
        if (tx < 0 || ty < 0 || tx >= width || ty >= height) {
          continue;
        }
        Pixel& out = pixels[static_cast<std::size_t>(ty * width + tx)];
        if (t.a == 255) {
          out = Pixel{static_cast<int>(t.r), static_cast<int>(t.g),
                      static_cast<int>(t.b)};
        } else {
          const int a = static_cast<int>(t.a);
          out.r = (static_cast<int>(t.r) * a + out.r * (255 - a)) / 255;
          out.g = (static_cast<int>(t.g) * a + out.g * (255 - a)) / 255;
          out.b = (static_cast<int>(t.b) * a + out.b * (255 - a)) / 255;
        }
      }
    }
  }

  Pixel ReadPixelShaderSource(int x, int y) const {
    x = std::max(0, std::min(width - 1, x));
    y = std::max(0, std::min(height - 1, y));
    return pixels[static_cast<std::size_t>(y * width + x)];
  }

  Pixel ApplyShaderOp(int mode, int p1, int p2, int p3, int x, int y,
                      Pixel p) const {
    const double pi = 3.14159265358979323846;
    const int mix = std::max(0, std::min(1000, p1));
    if (mode == 1) {
      int lum = (p.r * 30 + p.g * 59 + p.b * 11) / 100;
      p.r = (p.r * (1000 - mix) + lum * mix) / 1000;
      p.g = (p.g * (1000 - mix) + lum * mix) / 1000;
      p.b = (p.b * (1000 - mix) + lum * mix) / 1000;
    } else if (mode == 2) {
      const int dark = std::max(0, std::min(255, p1));
      if (((y + present_frame) & 1) != 0) {
        p.r = (p.r * (255 - dark)) / 255;
        p.g = (p.g * (255 - dark)) / 255;
        p.b = (p.b * (255 - dark)) / 255;
      }
    } else if (mode == 3) {
      const int amp = std::max(0, std::min(64, p1));
      const double freq = std::max(1, p2) / 1000.0;
      const double speed = static_cast<double>(p3);
      const double phase = (static_cast<double>(y) * freq) +
                           (static_cast<double>(present_frame) * speed / 60.0);
      const int offset = static_cast<int>(
          std::round(std::sin(phase * pi) * static_cast<double>(amp)));
      p = ReadPixelShaderSource(x + offset, y);
    } else if (mode == 4) {
      const int inv_r = 255 - p.r;
      const int inv_g = 255 - p.g;
      const int inv_b = 255 - p.b;
      p.r = (p.r * (1000 - mix) + inv_r * mix) / 1000;
      p.g = (p.g * (1000 - mix) + inv_g * mix) / 1000;
      p.b = (p.b * (1000 - mix) + inv_b * mix) / 1000;
    } else if (mode == 5) {
      int levels = p1;
      if (levels < 2) levels = 2;
      if (levels > 64) levels = 64;
      const int step = std::max(1, 255 / (levels - 1));
      p.r = ((p.r + step / 2) / step) * step;
      p.g = ((p.g + step / 2) / step) * step;
      p.b = ((p.b + step / 2) / step) * step;
    } else if (mode == 6) {
      const int off = std::max(0, std::min(24, p1));
      const Pixel pr = ReadPixelShaderSource(x - off, y);
      const Pixel pg = ReadPixelShaderSource(x, y);
      const Pixel pb = ReadPixelShaderSource(x + off, y);
      p.r = pr.r;
      p.g = pg.g;
      p.b = pb.b;
    } else if (mode == 7) {
      const int strength = std::max(0, std::min(255, p1));
      const double cx = static_cast<double>(width) * 0.5;
      const double cy = static_cast<double>(height) * 0.5;
      const double nx = (static_cast<double>(x) - cx) / cx;
      const double ny = (static_cast<double>(y) - cy) / cy;
      double d = std::sqrt(nx * nx + ny * ny);
      if (d > 1.0) d = 1.0;
      const int dark = static_cast<int>(std::round(d * strength));
      p.r = (p.r * (255 - dark)) / 255;
      p.g = (p.g * (255 - dark)) / 255;
      p.b = (p.b * (255 - dark)) / 255;
    }
    p.r = ClampColor(p.r);
    p.g = ClampColor(p.g);
    p.b = ClampColor(p.b);
    return p;
  }

  void BuildPresentBuffer() {
    const bool has_program =
        (shader_program_active >= 0 &&
         static_cast<std::size_t>(shader_program_active) < shader_programs.size() &&
         !shader_programs[static_cast<std::size_t>(shader_program_active)]
              .ops.empty());
    const std::vector<ShaderOp>* ops =
        has_program ? &shader_programs[static_cast<std::size_t>(shader_program_active)].ops
                    : nullptr;

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        Pixel p = ReadPixelShaderSource(x, y);
        if (ops != nullptr) {
          for (const ShaderOp& op : *ops) {
            p = ApplyShaderOp(op.mode, op.p1, op.p2, op.p3, x, y, p);
          }
        } else if (shader_mode != 0) {
          p = ApplyShaderOp(shader_mode, shader_p1, shader_p2, shader_p3, x, y,
                            p);
        }
        std::uint32_t value = (static_cast<std::uint32_t>(p.b) << 16) |
                              (static_cast<std::uint32_t>(p.g) << 8) |
                              static_cast<std::uint32_t>(p.r);
        rgba_buffer[static_cast<std::size_t>(y * width + x)] = value;
      }
    }
  }

  using Glyph5x7 = std::array<std::string_view, 7>;

  static const Glyph5x7& GlyphForChar(char raw) {
    const unsigned char uc = static_cast<unsigned char>(raw);
    const char c = static_cast<char>(std::toupper(uc));

    static const Glyph5x7 kBlank = {".....", ".....", ".....", ".....", ".....",
                                    ".....", "....."};
    static const Glyph5x7 kUnknown = {"XXXXX", "X...X", "...X.", "..X..", "..X..",
                                      ".....", "..X.."};
    static const Glyph5x7 k0 = {".XXX.", "X...X", "X..XX", "X.X.X", "XX..X",
                                "X...X", ".XXX."};
    static const Glyph5x7 k1 = {"..X..", ".XX..", "..X..", "..X..", "..X..",
                                "..X..", ".XXX."};
    static const Glyph5x7 k2 = {".XXX.", "X...X", "....X", "...X.", "..X..",
                                ".X...", "XXXXX"};
    static const Glyph5x7 k3 = {"XXXXX", "....X", "...X.", "..XX.", "....X",
                                "X...X", ".XXX."};
    static const Glyph5x7 k4 = {"...X.", "..XX.", ".X.X.", "X..X.", "XXXXX",
                                "...X.", "...X."};
    static const Glyph5x7 k5 = {"XXXXX", "X....", "XXXX.", "....X", "....X",
                                "X...X", ".XXX."};
    static const Glyph5x7 k6 = {".XXX.", "X...X", "X....", "XXXX.", "X...X",
                                "X...X", ".XXX."};
    static const Glyph5x7 k7 = {"XXXXX", "....X", "...X.", "..X..", ".X...",
                                ".X...", ".X..."};
    static const Glyph5x7 k8 = {".XXX.", "X...X", "X...X", ".XXX.", "X...X",
                                "X...X", ".XXX."};
    static const Glyph5x7 k9 = {".XXX.", "X...X", "X...X", ".XXXX", "....X",
                                "X...X", ".XXX."};
    static const Glyph5x7 kA = {".XXX.", "X...X", "X...X", "XXXXX", "X...X",
                                "X...X", "X...X"};
    static const Glyph5x7 kB = {"XXXX.", "X...X", "X...X", "XXXX.", "X...X",
                                "X...X", "XXXX."};
    static const Glyph5x7 kC = {".XXX.", "X...X", "X....", "X....", "X....",
                                "X...X", ".XXX."};
    static const Glyph5x7 kD = {"XXXX.", "X...X", "X...X", "X...X", "X...X",
                                "X...X", "XXXX."};
    static const Glyph5x7 kE = {"XXXXX", "X....", "X....", "XXXX.", "X....",
                                "X....", "XXXXX"};
    static const Glyph5x7 kF = {"XXXXX", "X....", "X....", "XXXX.", "X....",
                                "X....", "X...."};
    static const Glyph5x7 kG = {".XXX.", "X...X", "X....", "X.XXX", "X...X",
                                "X...X", ".XXX."};
    static const Glyph5x7 kH = {"X...X", "X...X", "X...X", "XXXXX", "X...X",
                                "X...X", "X...X"};
    static const Glyph5x7 kI = {".XXX.", "..X..", "..X..", "..X..", "..X..",
                                "..X..", ".XXX."};
    static const Glyph5x7 kJ = {"..XXX", "...X.", "...X.", "...X.", "...X.",
                                "X..X.", ".XX.."};
    static const Glyph5x7 kK = {"X...X", "X..X.", "X.X..", "XX...", "X.X..",
                                "X..X.", "X...X"};
    static const Glyph5x7 kL = {"X....", "X....", "X....", "X....", "X....",
                                "X....", "XXXXX"};
    static const Glyph5x7 kM = {"X...X", "XX.XX", "X.X.X", "X...X", "X...X",
                                "X...X", "X...X"};
    static const Glyph5x7 kN = {"X...X", "XX..X", "X.X.X", "X..XX", "X...X",
                                "X...X", "X...X"};
    static const Glyph5x7 kO = {".XXX.", "X...X", "X...X", "X...X", "X...X",
                                "X...X", ".XXX."};
    static const Glyph5x7 kP = {"XXXX.", "X...X", "X...X", "XXXX.", "X....",
                                "X....", "X...."};
    static const Glyph5x7 kQ = {".XXX.", "X...X", "X...X", "X...X", "X.X.X",
                                "X..X.", ".XX.X"};
    static const Glyph5x7 kR = {"XXXX.", "X...X", "X...X", "XXXX.", "X.X..",
                                "X..X.", "X...X"};
    static const Glyph5x7 kS = {".XXXX", "X....", "X....", ".XXX.", "....X",
                                "....X", "XXXX."};
    static const Glyph5x7 kT = {"XXXXX", "..X..", "..X..", "..X..", "..X..",
                                "..X..", "..X.."};
    static const Glyph5x7 kU = {"X...X", "X...X", "X...X", "X...X", "X...X",
                                "X...X", ".XXX."};
    static const Glyph5x7 kV = {"X...X", "X...X", "X...X", "X...X", "X...X",
                                ".X.X.", "..X.."};
    static const Glyph5x7 kW = {"X...X", "X...X", "X...X", "X.X.X", "X.X.X",
                                "XX.XX", "X...X"};
    static const Glyph5x7 kX = {"X...X", "X...X", ".X.X.", "..X..", ".X.X.",
                                "X...X", "X...X"};
    static const Glyph5x7 kY = {"X...X", "X...X", ".X.X.", "..X..", "..X..",
                                "..X..", "..X.."};
    static const Glyph5x7 kZ = {"XXXXX", "....X", "...X.", "..X..", ".X...",
                                "X....", "XXXXX"};
    static const Glyph5x7 kColon = {".....", "..X..", ".....", ".....", "..X..",
                                    ".....", "....."};
    static const Glyph5x7 kDot = {".....", ".....", ".....", ".....", ".....",
                                  "..X..", "....."};
    static const Glyph5x7 kEx = {"..X..", "..X..", "..X..", "..X..", "..X..",
                                 ".....", "..X.."};
    static const Glyph5x7 kDash = {".....", ".....", ".....", ".XXX.", ".....",
                                   ".....", "....."};
    static const Glyph5x7 kPlus = {".....", "..X..", "..X..", "XXXXX", "..X..",
                                   "..X..", "....."};
    static const Glyph5x7 kSlash = {"....X", "...X.", "..X..", ".X...", "X....",
                                    ".....", "....."};
    static const Glyph5x7 kLParen = {"...X.", "..X..", ".X...", ".X...", ".X...",
                                     "..X..", "...X."};
    static const Glyph5x7 kRParen = {".X...", "..X..", "...X.", "...X.", "...X.",
                                     "..X..", ".X..."};

    if (c == ' ') return kBlank;
    if (c == '0') return k0;
    if (c == '1') return k1;
    if (c == '2') return k2;
    if (c == '3') return k3;
    if (c == '4') return k4;
    if (c == '5') return k5;
    if (c == '6') return k6;
    if (c == '7') return k7;
    if (c == '8') return k8;
    if (c == '9') return k9;
    if (c == 'A') return kA;
    if (c == 'B') return kB;
    if (c == 'C') return kC;
    if (c == 'D') return kD;
    if (c == 'E') return kE;
    if (c == 'F') return kF;
    if (c == 'G') return kG;
    if (c == 'H') return kH;
    if (c == 'I') return kI;
    if (c == 'J') return kJ;
    if (c == 'K') return kK;
    if (c == 'L') return kL;
    if (c == 'M') return kM;
    if (c == 'N') return kN;
    if (c == 'O') return kO;
    if (c == 'P') return kP;
    if (c == 'Q') return kQ;
    if (c == 'R') return kR;
    if (c == 'S') return kS;
    if (c == 'T') return kT;
    if (c == 'U') return kU;
    if (c == 'V') return kV;
    if (c == 'W') return kW;
    if (c == 'X') return kX;
    if (c == 'Y') return kY;
    if (c == 'Z') return kZ;
    if (c == ':') return kColon;
    if (c == '.') return kDot;
    if (c == '!') return kEx;
    if (c == '-') return kDash;
    if (c == '+') return kPlus;
    if (c == '/') return kSlash;
    if (c == '(') return kLParen;
    if (c == ')') return kRParen;
    return kUnknown;
  }

  void DrawGlyph5x7(int x, int y, char c, const Pixel& color) {
    const Glyph5x7& glyph = GlyphForChar(c);
    for (int row = 0; row < 7; ++row) {
      for (int col = 0; col < 5; ++col) {
        if (glyph[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)] !=
            '.') {
          SetPixelRaw(x + col, y + row, color);
        }
      }
    }
  }

#ifdef _WIN32
  struct GdiPlusRuntime {
    ULONG_PTR token = 0;
    GdiPlusRuntime() {
      Gdiplus::GdiplusStartupInput input;
      Gdiplus::Status st = Gdiplus::GdiplusStartup(&token, &input, nullptr);
      if (st != Gdiplus::Ok) {
        throw std::runtime_error("Failed to initialize GDI+");
      }
    }
    ~GdiPlusRuntime() {
      if (token != 0) {
        Gdiplus::GdiplusShutdown(token);
      }
    }
  };

  static GdiPlusRuntime& GetGdiPlusRuntime() {
    static GdiPlusRuntime runtime;
    return runtime;
  }

  static void SetMouseVisibleImpl(bool visible) {
    CURSORINFO ci{};
    ci.cbSize = sizeof(CURSORINFO);
    if (!GetCursorInfo(&ci)) {
      return;
    }
    bool currently_visible = (ci.flags & CURSOR_SHOWING) != 0;
    if (currently_visible == visible) {
      return;
    }
    if (visible) {
      while (ShowCursor(TRUE) < 0) {
      }
    } else {
      while (ShowCursor(FALSE) >= 0) {
      }
    }
  }

  void UpdateViewportRect() {
    if (!hwnd) {
      viewport_rect = RECT{0, 0, width, height};
      return;
    }
    RECT client{};
    if (!GetClientRect(hwnd, &client)) {
      viewport_rect = RECT{0, 0, width, height};
      return;
    }
    const int cw = std::max(1, static_cast<int>(client.right - client.left));
    const int ch = std::max(1, static_cast<int>(client.bottom - client.top));
    if (!keep_aspect) {
      viewport_rect = RECT{0, 0, cw, ch};
      return;
    }

    const int rw = aspect_w > 0 ? aspect_w : width;
    const int rh = aspect_h > 0 ? aspect_h : height;
    long long lhs = static_cast<long long>(cw) * static_cast<long long>(rh);
    long long rhs = static_cast<long long>(ch) * static_cast<long long>(rw);
    int vw = cw;
    int vh = ch;
    if (lhs > rhs) {
      vw = static_cast<int>((static_cast<long long>(ch) * rw) / rh);
      vh = ch;
    } else {
      vw = cw;
      vh = static_cast<int>((static_cast<long long>(cw) * rh) / rw);
    }
    const int ox = (cw - vw) / 2;
    const int oy = (ch - vh) / 2;
    viewport_rect = RECT{ox, oy, ox + vw, oy + vh};
  }

  void BlitFrameToHdc(HDC hdc) {
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    UpdateViewportRect();
    RECT client_rect{};
    if (GetClientRect(hwnd, &client_rect)) {
      const int cw =
          std::max(1, static_cast<int>(client_rect.right - client_rect.left));
      const int ch =
          std::max(1, static_cast<int>(client_rect.bottom - client_rect.top));
      const int vx = viewport_rect.left;
      const int vy = viewport_rect.top;
      const int vw =
          std::max(1, static_cast<int>(viewport_rect.right - viewport_rect.left));
      const int vh = std::max(
          1, static_cast<int>(viewport_rect.bottom - viewport_rect.top));
      HBRUSH black = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));

      if (vy > 0) {
        RECT top{0, 0, cw, vy};
        FillRect(hdc, &top, black);
      }
      if (vy + vh < ch) {
        RECT bottom{0, vy + vh, cw, ch};
        FillRect(hdc, &bottom, black);
      }
      if (vx > 0) {
        RECT left{0, vy, vx, vy + vh};
        FillRect(hdc, &left, black);
      }
      if (vx + vw < cw) {
        RECT right{vx + vw, vy, cw, vy + vh};
        FillRect(hdc, &right, black);
      }
    }

    SetStretchBltMode(hdc, COLORONCOLOR);
    const int dst_x = viewport_rect.left;
    const int dst_y = viewport_rect.top;
    const int dst_w =
        std::max(1, static_cast<int>(viewport_rect.right - viewport_rect.left));
    const int dst_h = std::max(
        1, static_cast<int>(viewport_rect.bottom - viewport_rect.top));
    StretchDIBits(hdc, dst_x, dst_y, dst_w, dst_h, 0, 0, width, height,
                  rgba_buffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
  }

  static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wparam,
                                        LPARAM lparam) {
    GraphicsState* state = nullptr;
    if (msg == WM_NCCREATE) {
      CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
      state = reinterpret_cast<GraphicsState*>(cs->lpCreateParams);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
    } else {
      state = reinterpret_cast<GraphicsState*>(
          GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (state) {
      return state->WndProc(hwnd, msg, wparam, lparam);
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
  }

  LRESULT WndProc(HWND hwnd_handle, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
      case WM_CLOSE:
        window_open = false;
        SetMouseVisibleImpl(true);
        DestroyWindow(hwnd_handle);
        return 0;
      case WM_DESTROY:
        window_open = false;
        SetMouseVisibleImpl(true);
        return 0;
      case WM_SIZE:
        UpdateViewportRect();
        return 0;
      case WM_ERASEBKGND:
        return 1;
      case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd_handle, &ps);
        if (window_open && !rgba_buffer.empty() && width > 0 && height > 0) {
          BlitFrameToHdc(hdc);
        }
        EndPaint(hwnd_handle, &ps);
        return 0;
      }
      case WM_SIZING:
        if (keep_aspect) {
          RECT* rc = reinterpret_cast<RECT*>(lparam);
          if (!rc) {
            return TRUE;
          }
          const int rw = aspect_w > 0 ? aspect_w : width;
          const int rh = aspect_h > 0 ? aspect_h : height;
          const int border_w = (rc->right - rc->left) - width;
          const int border_h = (rc->bottom - rc->top) - height;
          int client_w =
              std::max(1, static_cast<int>((rc->right - rc->left) - border_w));
          int client_h =
              std::max(1, static_cast<int>((rc->bottom - rc->top) - border_h));

          if ((wparam == WMSZ_LEFT) || (wparam == WMSZ_RIGHT) ||
              (wparam == WMSZ_TOPLEFT) || (wparam == WMSZ_TOPRIGHT) ||
              (wparam == WMSZ_BOTTOMLEFT) || (wparam == WMSZ_BOTTOMRIGHT)) {
            client_h = std::max(1, static_cast<int>(
                                       (static_cast<long long>(client_w) * rh) /
                                       rw));
          } else {
            client_w = std::max(1, static_cast<int>(
                                       (static_cast<long long>(client_h) * rw) /
                                       rh));
          }
          const int outer_w = client_w + border_w;
          const int outer_h = client_h + border_h;
          if (wparam == WMSZ_LEFT || wparam == WMSZ_TOPLEFT ||
              wparam == WMSZ_BOTTOMLEFT) {
            rc->left = rc->right - outer_w;
          } else {
            rc->right = rc->left + outer_w;
          }
          if (wparam == WMSZ_TOP || wparam == WMSZ_TOPLEFT ||
              wparam == WMSZ_TOPRIGHT) {
            rc->top = rc->bottom - outer_h;
          } else {
            rc->bottom = rc->top + outer_h;
          }
          return TRUE;
        }
        return TRUE;
      case WM_KEYDOWN:
        if (wparam < 256) {
          key_state[static_cast<std::size_t>(wparam)] = true;
        }
        return 0;
      case WM_KEYUP:
        if (wparam < 256) {
          key_state[static_cast<std::size_t>(wparam)] = false;
        }
        return 0;
      case WM_MOUSEMOVE: {
        const int mx = static_cast<int>(static_cast<short>(LOWORD(lparam)));
        const int my = static_cast<int>(static_cast<short>(HIWORD(lparam)));
        if (!suppress_mouse_delta && mouse_client_x >= 0 && mouse_client_y >= 0) {
          mouse_dx_acc += (mx - mouse_client_x);
          mouse_dy_acc += (my - mouse_client_y);
        }
        mouse_client_x = mx;
        mouse_client_y = my;
        suppress_mouse_delta = false;
        return 0;
      }
      case WM_LBUTTONDOWN:
        mouse_left_down = true;
        SetCapture(hwnd_handle);
        return 0;
      case WM_LBUTTONUP:
        mouse_left_down = false;
        ReleaseCapture();
        return 0;
      case WM_RBUTTONDOWN:
        mouse_right_down = true;
        SetCapture(hwnd_handle);
        return 0;
      case WM_RBUTTONUP:
        mouse_right_down = false;
        ReleaseCapture();
        return 0;
      case WM_MBUTTONDOWN:
        mouse_middle_down = true;
        SetCapture(hwnd_handle);
        return 0;
      case WM_MBUTTONUP:
        mouse_middle_down = false;
        ReleaseCapture();
        return 0;
      default:
        return DefWindowProcW(hwnd_handle, msg, wparam, lparam);
    }
  }
#endif
};

std::vector<Instruction> CompileSource(const std::filesystem::path& source_file);

class VM {
 public:
  explicit VM(std::filesystem::path module_base = std::filesystem::current_path())
      : module_base_(std::move(module_base)) {}

  void Execute(const std::vector<Instruction>& code) {
    std::size_t ip = 0;
    while (ip < code.size()) {
      const Instruction& ins = code[ip];
      if (ins.op == "HALT") {
        return;
      }
      if (ins.op == "PUSH_INT") {
        stack_.push_back(std::stoi(ins.args[0]));
      } else if (ins.op == "PUSH_STR") {
        stack_.push_back(ins.args[0]);
      } else if (ins.op == "LOAD") {
        auto it = vars_.find(ins.args[0]);
        if (it == vars_.end()) {
          throw std::runtime_error("Undefined variable: " + ins.args[0]);
        }
        stack_.push_back(it->second);
      } else if (ins.op == "STORE") {
        Value value = Pop();
        vars_[ins.args[0]] = value;
      } else if (ins.op == "NEW_OBJ") {
        stack_.push_back(std::make_shared<Object>());
      } else if (ins.op == "SET_FIELD") {
        if (ins.args.empty()) {
          throw std::runtime_error("SET_FIELD missing field name");
        }
        Value value = Pop();
        Value objv = Pop();
        if (!std::holds_alternative<ObjectPtr>(objv) ||
            !std::get<ObjectPtr>(objv)) {
          throw std::runtime_error("SET_FIELD expects object");
        }
        ObjectPtr obj = std::get<ObjectPtr>(objv);
        obj->fields[ins.args[0]] = value;
        stack_.push_back(obj);
      } else if (ins.op == "GET_FIELD") {
        if (ins.args.empty()) {
          throw std::runtime_error("GET_FIELD missing field name");
        }
        Value objv = Pop();
        if (!std::holds_alternative<ObjectPtr>(objv) ||
            !std::get<ObjectPtr>(objv)) {
          throw std::runtime_error("GET_FIELD expects object");
        }
        ObjectPtr obj = std::get<ObjectPtr>(objv);
        auto it = obj->fields.find(ins.args[0]);
        if (it == obj->fields.end()) {
          throw std::runtime_error("Unknown object field: " + ins.args[0]);
        }
        stack_.push_back(it->second);
      } else if (ins.op == "POP") {
        (void)Pop();
      } else if (ins.op == "NEG") {
        int value = ValueAsInt(Pop(), "NEG");
        stack_.push_back(-value);
      } else if (ins.op == "ADD" || ins.op == "SUB" || ins.op == "MUL" ||
                 ins.op == "DIV") {
        RunArithmetic(ins.op);
      } else if (ins.op == "CMP_EQ" || ins.op == "CMP_NE" || ins.op == "CMP_LT" ||
                 ins.op == "CMP_LE" || ins.op == "CMP_GT" || ins.op == "CMP_GE") {
        RunComparison(ins.op);
      } else if (ins.op == "JZ") {
        int target = std::stoi(ins.args[0]);
        if (!ValueIsTruthy(Pop())) {
          if (target < 0 || static_cast<std::size_t>(target) >= code.size()) {
            throw std::runtime_error("Invalid jump target");
          }
          ip = static_cast<std::size_t>(target);
          continue;
        }
      } else if (ins.op == "JMP") {
        int target = std::stoi(ins.args[0]);
        if (target < 0 || static_cast<std::size_t>(target) >= code.size()) {
          throw std::runtime_error("Invalid jump target");
        }
        ip = static_cast<std::size_t>(target);
        continue;
      } else if (ins.op == "CALL") {
        RunCall(ins.args[0], std::stoi(ins.args[1]));
      } else if (ins.op == "IMPORT") {
        RunImport(ins.args[0], ins.args[1]);
      } else {
        throw std::runtime_error("Unknown opcode: " + ins.op);
      }
      ip += 1;
    }
  }

  const std::unordered_map<std::string, Value>& Globals() const { return vars_; }

 private:
  class Gx3dState {
   public:
    struct ScreenVertex {
      int x = 0;
      int y = 0;
      double z = 0.0;
    };
    struct ScreenVertexUv {
      int x = 0;
      int y = 0;
      double z = 0.0;
      double u = 0.0;
      double v = 0.0;
    };

    explicit Gx3dState(GraphicsState& gfx) : gfx_(gfx) {}

    void Reset() {
      cam_ = Vec3{0.0, 0.0, -220.0};
      rot_deg_ = Vec3{0.0, 0.0, 0.0};
      trans_ = Vec3{0.0, 0.0, 0.0};
      scale_ = Vec3{1.0, 1.0, 1.0};
      fov_ = 300.0;
      near_clip_ = 1.0;
      far_clip_ = 10000.0;
      depth_dirty_ = true;
    }

    void OnFrameReset() { depth_dirty_ = true; }

    void Camera(int x, int y, int z) {
      cam_ = Vec3{static_cast<double>(x), static_cast<double>(y),
                  static_cast<double>(z)};
    }

    void CameraMove(int dx, int dy, int dz) {
      cam_.x += static_cast<double>(dx);
      cam_.y += static_cast<double>(dy);
      cam_.z += static_cast<double>(dz);
    }

    void Rotate(int x_deg, int y_deg, int z_deg) {
      rot_deg_ = Vec3{static_cast<double>(x_deg), static_cast<double>(y_deg),
                      static_cast<double>(z_deg)};
    }

    void RotateAdd(int dx_deg, int dy_deg, int dz_deg) {
      rot_deg_.x += static_cast<double>(dx_deg);
      rot_deg_.y += static_cast<double>(dy_deg);
      rot_deg_.z += static_cast<double>(dz_deg);
    }

    void Translate(int x, int y, int z) {
      trans_ = Vec3{static_cast<double>(x), static_cast<double>(y),
                    static_cast<double>(z)};
    }

    void Scale(int sx, int sy, int sz) {
      if (sx <= 0 || sy <= 0 || sz <= 0) {
        throw std::runtime_error("gx3d.scale expects positive values");
      }
      scale_ = Vec3{static_cast<double>(sx) / 1000.0,
                    static_cast<double>(sy) / 1000.0,
                    static_cast<double>(sz) / 1000.0};
    }

    void ScaleUniform(int s) {
      Scale(s, s, s);
    }

    void Fov(int fov) {
      if (fov <= 10) {
        throw std::runtime_error("gx3d.fov expects value > 10");
      }
      fov_ = static_cast<double>(fov);
    }

    void Clip(int near_z, int far_z) {
      if (near_z <= 0 || far_z <= near_z) {
        throw std::runtime_error("gx3d.clip expects near>0 and far>near");
      }
      near_clip_ = static_cast<double>(near_z);
      far_clip_ = static_cast<double>(far_z);
    }

    void Point(int x, int y, int z, int r, int g, int b) {
      RequireGfx("gx3d.point");
      auto p = Project(ApplyTransform(Vec3{static_cast<double>(x),
                                           static_cast<double>(y),
                                           static_cast<double>(z)}));
      if (p.has_value()) {
        gfx_.PixelAt(p->first, p->second, ClampColor(r), ClampColor(g),
                     ClampColor(b));
      }
    }

    void Line3d(int x1, int y1, int z1, int x2, int y2, int z2, int r, int g,
                int b) {
      RequireGfx("gx3d.line");
      auto p1 = Project(ApplyTransform(Vec3{static_cast<double>(x1),
                                            static_cast<double>(y1),
                                            static_cast<double>(z1)}));
      auto p2 = Project(ApplyTransform(Vec3{static_cast<double>(x2),
                                            static_cast<double>(y2),
                                            static_cast<double>(z2)}));
      if (p1.has_value() && p2.has_value()) {
        gfx_.Line(p1->first, p1->second, p2->first, p2->second, ClampColor(r),
                  ClampColor(g), ClampColor(b));
      }
    }

    void Cube(int cx, int cy, int cz, int size, int r, int g, int b) {
      RequireGfx("gx3d.cube");
      if (size <= 0) {
        return;
      }

      const double h = static_cast<double>(size) / 2.0;
      std::array<Vec3, 8> verts = {
          Vec3{-h, -h, -h}, Vec3{h, -h, -h},  Vec3{h, h, -h},  Vec3{-h, h, -h},
          Vec3{-h, -h, h},  Vec3{h, -h, h},   Vec3{h, h, h},   Vec3{-h, h, h},
      };

      for (Vec3& v : verts) {
        v = ApplyTransform(v);
        v.x += static_cast<double>(cx);
        v.y += static_cast<double>(cy);
        v.z += static_cast<double>(cz);
      }

      static const std::array<std::pair<int, int>, 12> edges = {
          std::pair<int, int>{0, 1}, {1, 2}, {2, 3}, {3, 0},
          {4, 5},                    {5, 6}, {6, 7}, {7, 4},
          {0, 4},                    {1, 5}, {2, 6}, {3, 7},
      };

      for (const auto& [a, c] : edges) {
        auto p1 = Project(verts[static_cast<std::size_t>(a)]);
        auto p2 = Project(verts[static_cast<std::size_t>(c)]);
        if (p1.has_value() && p2.has_value()) {
          gfx_.Line(p1->first, p1->second, p2->first, p2->second, ClampColor(r),
                    ClampColor(g), ClampColor(b));
        }
      }
    }

    void Cuboid(int cx, int cy, int cz, int sx, int sy, int sz, int r, int g,
                int b) {
      RequireGfx("gx3d.cuboid");
      if (sx <= 0 || sy <= 0 || sz <= 0) {
        return;
      }
      const double hx = static_cast<double>(sx) / 2.0;
      const double hy = static_cast<double>(sy) / 2.0;
      const double hz = static_cast<double>(sz) / 2.0;
      std::array<Vec3, 8> verts = {
          Vec3{-hx, -hy, -hz}, Vec3{hx, -hy, -hz},  Vec3{hx, hy, -hz},
          Vec3{-hx, hy, -hz},  Vec3{-hx, -hy, hz},  Vec3{hx, -hy, hz},
          Vec3{hx, hy, hz},    Vec3{-hx, hy, hz},
      };
      for (Vec3& v : verts) {
        v = ApplyTransform(v);
        v.x += static_cast<double>(cx);
        v.y += static_cast<double>(cy);
        v.z += static_cast<double>(cz);
      }
      static const std::array<std::pair<int, int>, 12> edges = {
          std::pair<int, int>{0, 1}, {1, 2}, {2, 3}, {3, 0},
          {4, 5},                    {5, 6}, {6, 7}, {7, 4},
          {0, 4},                    {1, 5}, {2, 6}, {3, 7},
      };
      for (const auto& [a, c] : edges) {
        auto p1 = Project(verts[static_cast<std::size_t>(a)]);
        auto p2 = Project(verts[static_cast<std::size_t>(c)]);
        if (p1.has_value() && p2.has_value()) {
          gfx_.Line(p1->first, p1->second, p2->first, p2->second, ClampColor(r),
                    ClampColor(g), ClampColor(b));
        }
      }
    }

    void CubeSolid(int cx, int cy, int cz, int size, int r, int g, int b) {
      if (size <= 0) {
        return;
      }
      CuboidSolid(cx, cy, cz, size, size, size, r, g, b);
    }

    void Pyramid(int cx, int cy, int cz, int size, int r, int g, int b) {
      RequireGfx("gx3d.pyramid");
      if (size <= 0) {
        return;
      }
      const double h = static_cast<double>(size) / 2.0;
      std::array<Vec3, 5> verts = {
          Vec3{-h, -h, -h}, Vec3{h, -h, -h}, Vec3{h, -h, h},
          Vec3{-h, -h, h},  Vec3{0.0, h, 0.0},
      };
      for (Vec3& v : verts) {
        v = ApplyTransform(v);
        v.x += static_cast<double>(cx);
        v.y += static_cast<double>(cy);
        v.z += static_cast<double>(cz);
      }

      static const std::array<std::pair<int, int>, 8> edges = {
          std::pair<int, int>{0, 1}, {1, 2}, {2, 3}, {3, 0},
          {0, 4},                    {1, 4}, {2, 4}, {3, 4},
      };
      for (const auto& [a, c] : edges) {
        auto p1 = Project(verts[static_cast<std::size_t>(a)]);
        auto p2 = Project(verts[static_cast<std::size_t>(c)]);
        if (p1.has_value() && p2.has_value()) {
          gfx_.Line(p1->first, p1->second, p2->first, p2->second, ClampColor(r),
                    ClampColor(g), ClampColor(b));
        }
      }
    }

    void CuboidSolid(int cx, int cy, int cz, int sx, int sy, int sz, int r,
                     int g, int b) {
      RequireGfx("gx3d.cuboid_solid");
      if (sx <= 0 || sy <= 0 || sz <= 0) {
        return;
      }
      EnsureDepthBuffer();

      const double hx = static_cast<double>(sx) / 2.0;
      const double hy = static_cast<double>(sy) / 2.0;
      const double hz = static_cast<double>(sz) / 2.0;
      std::array<Vec3, 8> verts = {
          Vec3{-hx, -hy, -hz}, Vec3{hx, -hy, -hz},  Vec3{hx, hy, -hz},
          Vec3{-hx, hy, -hz},  Vec3{-hx, -hy, hz},  Vec3{hx, -hy, hz},
          Vec3{hx, hy, hz},    Vec3{-hx, hy, hz},
      };

      for (Vec3& v : verts) {
        v = ApplyTransform(v);
        v.x += static_cast<double>(cx);
        v.y += static_cast<double>(cy);
        v.z += static_cast<double>(cz);
      }

      static const std::array<std::array<int, 4>, 6> faces = {{
          {0, 1, 2, 3},
          {4, 5, 6, 7},
          {0, 1, 5, 4},
          {2, 3, 7, 6},
          {1, 2, 6, 5},
          {0, 3, 7, 4},
      }};

      for (const auto& f : faces) {
        auto sv0 = ProjectVertex(verts[static_cast<std::size_t>(f[0])]);
        auto sv1 = ProjectVertex(verts[static_cast<std::size_t>(f[1])]);
        auto sv2 = ProjectVertex(verts[static_cast<std::size_t>(f[2])]);
        auto sv3 = ProjectVertex(verts[static_cast<std::size_t>(f[3])]);
        if (!sv0.has_value() || !sv1.has_value() || !sv2.has_value() ||
            !sv3.has_value()) {
          continue;
        }

        int shade = 255 - static_cast<int>((sv0->z + sv1->z + sv2->z + sv3->z) / 4.0 / 40.0);
        if (shade < 55) shade = 55;
        if (shade > 255) shade = 255;
        const int sr = (ClampColor(r) * shade) / 255;
        const int sg = (ClampColor(g) * shade) / 255;
        const int sb = (ClampColor(b) * shade) / 255;

        FillTriangleDepth(*sv0, *sv1, *sv2, sr, sg, sb);
        FillTriangleDepth(*sv0, *sv2, *sv3, sr, sg, sb);
      }
    }

    void Triangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3,
                  int y3, int z3, int r, int g, int b) {
      RequireGfx("gx3d.triangle");
      auto p1 = Project(ApplyTransform(Vec3{static_cast<double>(x1),
                                            static_cast<double>(y1),
                                            static_cast<double>(z1)}));
      auto p2 = Project(ApplyTransform(Vec3{static_cast<double>(x2),
                                            static_cast<double>(y2),
                                            static_cast<double>(z2)}));
      auto p3 = Project(ApplyTransform(Vec3{static_cast<double>(x3),
                                            static_cast<double>(y3),
                                            static_cast<double>(z3)}));
      if (!p1.has_value() || !p2.has_value() || !p3.has_value()) {
        return;
      }
      gfx_.Line(p1->first, p1->second, p2->first, p2->second, ClampColor(r),
                ClampColor(g), ClampColor(b));
      gfx_.Line(p2->first, p2->second, p3->first, p3->second, ClampColor(r),
                ClampColor(g), ClampColor(b));
      gfx_.Line(p3->first, p3->second, p1->first, p1->second, ClampColor(r),
                ClampColor(g), ClampColor(b));
    }

    void TriangleSolid(int x1, int y1, int z1, int x2, int y2, int z2, int x3,
                       int y3, int z3, int r, int g, int b) {
      RequireGfx("gx3d.triangle_solid");
      EnsureDepthBuffer();
      auto sv1 = ProjectVertex(ApplyTransform(Vec3{static_cast<double>(x1),
                                                   static_cast<double>(y1),
                                                   static_cast<double>(z1)}));
      auto sv2 = ProjectVertex(ApplyTransform(Vec3{static_cast<double>(x2),
                                                   static_cast<double>(y2),
                                                   static_cast<double>(z2)}));
      auto sv3 = ProjectVertex(ApplyTransform(Vec3{static_cast<double>(x3),
                                                   static_cast<double>(y3),
                                                   static_cast<double>(z3)}));
      if (!sv1.has_value() || !sv2.has_value() || !sv3.has_value()) {
        return;
      }
      FillTriangleDepth(*sv1, *sv2, *sv3, ClampColor(r), ClampColor(g),
                        ClampColor(b));
    }

    void Quad(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3,
              int z3, int x4, int y4, int z4, int r, int g, int b) {
      RequireGfx("gx3d.quad");
      Triangle(x1, y1, z1, x2, y2, z2, x3, y3, z3, r, g, b);
      Triangle(x1, y1, z1, x3, y3, z3, x4, y4, z4, r, g, b);
    }

    void QuadSolid(int x1, int y1, int z1, int x2, int y2, int z2, int x3,
                   int y3, int z3, int x4, int y4, int z4, int r, int g,
                   int b) {
      RequireGfx("gx3d.quad_solid");
      TriangleSolid(x1, y1, z1, x2, y2, z2, x3, y3, z3, r, g, b);
      TriangleSolid(x1, y1, z1, x3, y3, z3, x4, y4, z4, r, g, b);
    }

    void Sphere(int cx, int cy, int cz, int radius, int segments, int r, int g,
                int b) {
      RequireGfx("gx3d.sphere");
      if (radius <= 0) {
        return;
      }
      if (segments < 4) {
        segments = 4;
      }
      if (segments > 64) {
        segments = 64;
      }
      const double pi = 3.14159265358979323846;
      auto draw_local_line = [&](const Vec3& a, const Vec3& c) {
        Vec3 ta = ApplyTransform(a);
        Vec3 tc = ApplyTransform(c);
        ta.x += static_cast<double>(cx);
        ta.y += static_cast<double>(cy);
        ta.z += static_cast<double>(cz);
        tc.x += static_cast<double>(cx);
        tc.y += static_cast<double>(cy);
        tc.z += static_cast<double>(cz);
        auto p1 = Project(ta);
        auto p2 = Project(tc);
        if (p1.has_value() && p2.has_value()) {
          gfx_.Line(p1->first, p1->second, p2->first, p2->second, ClampColor(r),
                    ClampColor(g), ClampColor(b));
        }
      };
      for (int lat = 1; lat < segments; ++lat) {
        const double t = static_cast<double>(lat) / static_cast<double>(segments);
        const double phi = -pi / 2.0 + pi * t;
        const double y = std::sin(phi) * static_cast<double>(radius);
        const double rr = std::cos(phi) * static_cast<double>(radius);
        for (int lon = 0; lon < segments; ++lon) {
          const double a0 = (2.0 * pi * static_cast<double>(lon)) /
                            static_cast<double>(segments);
          const double a1 = (2.0 * pi * static_cast<double>(lon + 1)) /
                            static_cast<double>(segments);
          const int x0 = static_cast<int>(std::round(std::cos(a0) * rr));
          const int z0 = static_cast<int>(std::round(std::sin(a0) * rr));
          const int x1 = static_cast<int>(std::round(std::cos(a1) * rr));
          const int z1 = static_cast<int>(std::round(std::sin(a1) * rr));
          draw_local_line(
              Vec3{static_cast<double>(x0), y, static_cast<double>(z0)},
              Vec3{static_cast<double>(x1), y, static_cast<double>(z1)});
        }
      }
      // Longitudinal arcs
      for (int lon = 0; lon < segments; ++lon) {
        const double a = (2.0 * pi * static_cast<double>(lon)) /
                         static_cast<double>(segments);
        int prev_x = 0;
        int prev_y = 0;
        int prev_z = 0;
        bool has_prev = false;
        for (int lat = 0; lat <= segments; ++lat) {
          const double t = static_cast<double>(lat) / static_cast<double>(segments);
          const double phi = -pi / 2.0 + pi * t;
          const double rr = std::cos(phi) * static_cast<double>(radius);
          const int x = static_cast<int>(std::round(std::cos(a) * rr));
          const int y = static_cast<int>(std::round(std::sin(phi) *
                                                    static_cast<double>(radius)));
          const int z = static_cast<int>(std::round(std::sin(a) * rr));
          if (has_prev) {
            draw_local_line(
                Vec3{static_cast<double>(prev_x), static_cast<double>(prev_y),
                     static_cast<double>(prev_z)},
                Vec3{static_cast<double>(x), static_cast<double>(y),
                     static_cast<double>(z)});
          }
          prev_x = x;
          prev_y = y;
          prev_z = z;
          has_prev = true;
        }
      }
    }

    void PyramidSolid(int cx, int cy, int cz, int size, int r, int g, int b) {
      RequireGfx("gx3d.pyramid_solid");
      if (size <= 0) {
        return;
      }
      EnsureDepthBuffer();

      const double h = static_cast<double>(size) / 2.0;
      std::array<Vec3, 5> verts = {
          Vec3{-h, -h, -h}, Vec3{h, -h, -h}, Vec3{h, -h, h},
          Vec3{-h, -h, h},  Vec3{0.0, h, 0.0},
      };
      for (Vec3& v : verts) {
        v = ApplyTransform(v);
        v.x += static_cast<double>(cx);
        v.y += static_cast<double>(cy);
        v.z += static_cast<double>(cz);
      }

      static const std::array<std::array<int, 3>, 6> faces = {{
          {0, 1, 4},
          {1, 2, 4},
          {2, 3, 4},
          {3, 0, 4},
          {0, 1, 2},
          {0, 2, 3},
      }};

      for (const auto& f : faces) {
        auto sv0 = ProjectVertex(verts[static_cast<std::size_t>(f[0])]);
        auto sv1 = ProjectVertex(verts[static_cast<std::size_t>(f[1])]);
        auto sv2 = ProjectVertex(verts[static_cast<std::size_t>(f[2])]);
        if (!sv0.has_value() || !sv1.has_value() || !sv2.has_value()) {
          continue;
        }
        int shade = 255 - static_cast<int>((sv0->z + sv1->z + sv2->z) / 3.0 / 45.0);
        if (shade < 55) shade = 55;
        if (shade > 255) shade = 255;
        const int sr = (ClampColor(r) * shade) / 255;
        const int sg = (ClampColor(g) * shade) / 255;
        const int sb = (ClampColor(b) * shade) / 255;
        FillTriangleDepth(*sv0, *sv1, *sv2, sr, sg, sb);
      }
    }

    void CubeSprite(int cx, int cy, int cz, int size, int sprite_id) {
      if (size <= 0) {
        return;
      }
      CuboidSprite(cx, cy, cz, size, size, size, sprite_id);
    }

    void CuboidSprite(int cx, int cy, int cz, int sx, int sy, int sz,
                      int sprite_id) {
      RequireGfx("gx3d.cuboid_sprite");
      if (sx <= 0 || sy <= 0 || sz <= 0) {
        return;
      }
      const GraphicsState::SpriteAsset& spr =
          gfx_.GetSpriteAsset(sprite_id, "gx3d.cuboid_sprite");
      if (spr.width <= 0 || spr.height <= 0) {
        return;
      }
      EnsureDepthBuffer();

      const double hx = static_cast<double>(sx) / 2.0;
      const double hy = static_cast<double>(sy) / 2.0;
      const double hz = static_cast<double>(sz) / 2.0;
      std::array<Vec3, 8> verts = {
          Vec3{-hx, -hy, -hz}, Vec3{hx, -hy, -hz},  Vec3{hx, hy, -hz},
          Vec3{-hx, hy, -hz},  Vec3{-hx, -hy, hz},  Vec3{hx, -hy, hz},
          Vec3{hx, hy, hz},    Vec3{-hx, hy, hz},
      };
      for (Vec3& v : verts) {
        v = ApplyTransform(v);
        v.x += static_cast<double>(cx);
        v.y += static_cast<double>(cy);
        v.z += static_cast<double>(cz);
      }

      static const std::array<std::array<int, 4>, 6> faces = {{
          {0, 1, 2, 3},
          {4, 5, 6, 7},
          {0, 1, 5, 4},
          {2, 3, 7, 6},
          {1, 2, 6, 5},
          {0, 3, 7, 4},
      }};

      static const std::array<std::pair<double, double>, 4> uv = {{
          {0.0, 0.0},
          {1.0, 0.0},
          {1.0, 1.0},
          {0.0, 1.0},
      }};

      for (const auto& f : faces) {
        auto s0 = ProjectVertexUv(verts[static_cast<std::size_t>(f[0])], uv[0].first,
                                  uv[0].second);
        auto s1 = ProjectVertexUv(verts[static_cast<std::size_t>(f[1])], uv[1].first,
                                  uv[1].second);
        auto s2 = ProjectVertexUv(verts[static_cast<std::size_t>(f[2])], uv[2].first,
                                  uv[2].second);
        auto s3 = ProjectVertexUv(verts[static_cast<std::size_t>(f[3])], uv[3].first,
                                  uv[3].second);
        if (!s0.has_value() || !s1.has_value() || !s2.has_value() ||
            !s3.has_value()) {
          continue;
        }

        FillTriangleDepthTextured(*s0, *s1, *s2, spr);
        FillTriangleDepthTextured(*s0, *s2, *s3, spr);
      }
    }

    void Axis(int len) {
      RequireGfx("gx3d.axis");
      if (len <= 0) {
        return;
      }
      Line3d(0, 0, 0, len, 0, 0, 255, 90, 90);
      Line3d(0, 0, 0, 0, len, 0, 90, 255, 90);
      Line3d(0, 0, 0, 0, 0, len, 90, 140, 255);
    }

    void Grid(int size, int step, int y) {
      RequireGfx("gx3d.grid");
      if (size <= 0 || step <= 0) {
        return;
      }
      for (int i = -size; i <= size; i += step) {
        Line3d(i, y, -size, i, y, size, 70, 80, 95);
        Line3d(-size, y, i, size, y, i, 70, 80, 95);
      }
    }

   private:
    static Vec3 RotateVec(Vec3 v, const Vec3& rot_deg) {
      const double rx = rot_deg.x * (3.14159265358979323846 / 180.0);
      const double ry = rot_deg.y * (3.14159265358979323846 / 180.0);
      const double rz = rot_deg.z * (3.14159265358979323846 / 180.0);

      const double cx = std::cos(rx);
      const double sx = std::sin(rx);
      const double cy = std::cos(ry);
      const double sy = std::sin(ry);
      const double cz = std::cos(rz);
      const double sz = std::sin(rz);

      // X rotation
      double y1 = v.y * cx - v.z * sx;
      double z1 = v.y * sx + v.z * cx;
      v.y = y1;
      v.z = z1;

      // Y rotation
      double x2 = v.x * cy + v.z * sy;
      double z2 = -v.x * sy + v.z * cy;
      v.x = x2;
      v.z = z2;

      // Z rotation
      double x3 = v.x * cz - v.y * sz;
      double y3 = v.x * sz + v.y * cz;
      v.x = x3;
      v.y = y3;
      return v;
    }

    Vec3 ApplyTransform(Vec3 v) const {
      v.x *= scale_.x;
      v.y *= scale_.y;
      v.z *= scale_.z;
      v = RotateVec(v, rot_deg_);
      v.x += trans_.x;
      v.y += trans_.y;
      v.z += trans_.z;
      return v;
    }

    std::optional<std::pair<int, int>> Project(const Vec3& world) const {
      auto sv = ProjectVertex(world);
      if (!sv.has_value()) {
        return std::nullopt;
      }
      return std::pair<int, int>{sv->x, sv->y};
    }

    std::optional<ScreenVertex> ProjectVertex(const Vec3& world) const {
      const double x = world.x - cam_.x;
      const double y = world.y - cam_.y;
      double z = world.z - cam_.z;

      // Reject points behind camera. Near-plane crossing points are clamped to
      // avoid face popping/flicker when objects move very close to the camera.
      if (z <= 0.0) {
        return std::nullopt;
      }
      if (z >= far_clip_) {
        return std::nullopt;
      }
      if (z < near_clip_) {
        z = near_clip_;
      }

      const double sx = (x / z) * fov_ + static_cast<double>(gfx_.Width()) / 2.0;
      const double sy = (-y / z) * fov_ + static_cast<double>(gfx_.Height()) / 2.0;
      return ScreenVertex{static_cast<int>(std::round(sx)),
                          static_cast<int>(std::round(sy)), z};
    }

    std::optional<ScreenVertexUv> ProjectVertexUv(const Vec3& world, double u,
                                                  double v) const {
      auto base = ProjectVertex(world);
      if (!base.has_value()) {
        return std::nullopt;
      }
      return ScreenVertexUv{base->x, base->y, base->z, u, v};
    }

    void EnsureDepthBuffer() {
      const std::size_t need =
          static_cast<std::size_t>(gfx_.Width() * gfx_.Height());
      if (depth_.size() != need) {
        depth_.assign(need, 1e30);
        depth_dirty_ = false;
        return;
      }
      if (depth_dirty_) {
        std::fill(depth_.begin(), depth_.end(), 1e30);
        depth_dirty_ = false;
      }
    }

    void FillTriangleDepth(const ScreenVertex& a, const ScreenVertex& b,
                           const ScreenVertex& c, int r, int g, int bl) {
      int min_x = std::min(a.x, std::min(b.x, c.x));
      int max_x = std::max(a.x, std::max(b.x, c.x));
      int min_y = std::min(a.y, std::min(b.y, c.y));
      int max_y = std::max(a.y, std::max(b.y, c.y));

      min_x = std::max(0, min_x);
      min_y = std::max(0, min_y);
      max_x = std::min(gfx_.Width() - 1, max_x);
      max_y = std::min(gfx_.Height() - 1, max_y);
      if (min_x > max_x || min_y > max_y) {
        return;
      }

      const double denom =
          static_cast<double>((b.y - c.y) * (a.x - c.x) +
                              (c.x - b.x) * (a.y - c.y));
      if (std::abs(denom) < 1e-9) {
        return;
      }

      for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
          const double px = static_cast<double>(x) + 0.5;
          const double py = static_cast<double>(y) + 0.5;
          const double w1 = ((b.y - c.y) * (px - c.x) + (c.x - b.x) * (py - c.y)) / denom;
          const double w2 = ((c.y - a.y) * (px - c.x) + (a.x - c.x) * (py - c.y)) / denom;
          const double w3 = 1.0 - w1 - w2;
          const bool inside_ccw = (w1 >= 0.0 && w2 >= 0.0 && w3 >= 0.0);
          const bool inside_cw = (w1 <= 0.0 && w2 <= 0.0 && w3 <= 0.0);
          if (!inside_ccw && !inside_cw) {
            continue;
          }

          const double z = w1 * a.z + w2 * b.z + w3 * c.z;
          const std::size_t idx =
              static_cast<std::size_t>(y * gfx_.Width() + x);
          if (z <= depth_[idx]) {
            depth_[idx] = z;
            gfx_.PixelAtFast(x, y, r, g, bl);
          }
        }
      }
    }

    void FillTriangleDepthTextured(const ScreenVertexUv& a,
                                   const ScreenVertexUv& b,
                                   const ScreenVertexUv& c,
                                   const GraphicsState::SpriteAsset& spr) {
      int min_x = std::min(a.x, std::min(b.x, c.x));
      int max_x = std::max(a.x, std::max(b.x, c.x));
      int min_y = std::min(a.y, std::min(b.y, c.y));
      int max_y = std::max(a.y, std::max(b.y, c.y));

      min_x = std::max(0, min_x);
      min_y = std::max(0, min_y);
      max_x = std::min(gfx_.Width() - 1, max_x);
      max_y = std::min(gfx_.Height() - 1, max_y);
      if (min_x > max_x || min_y > max_y) {
        return;
      }

      const double denom =
          static_cast<double>((b.y - c.y) * (a.x - c.x) +
                              (c.x - b.x) * (a.y - c.y));
      if (std::abs(denom) < 1e-9) {
        return;
      }

      for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
          const double px = static_cast<double>(x) + 0.5;
          const double py = static_cast<double>(y) + 0.5;
          const double w1 =
              ((b.y - c.y) * (px - c.x) + (c.x - b.x) * (py - c.y)) / denom;
          const double w2 =
              ((c.y - a.y) * (px - c.x) + (a.x - c.x) * (py - c.y)) / denom;
          const double w3 = 1.0 - w1 - w2;
          const bool inside_ccw = (w1 >= 0.0 && w2 >= 0.0 && w3 >= 0.0);
          const bool inside_cw = (w1 <= 0.0 && w2 <= 0.0 && w3 <= 0.0);
          if (!inside_ccw && !inside_cw) {
            continue;
          }

          const double z = w1 * a.z + w2 * b.z + w3 * c.z;
          const std::size_t idx =
              static_cast<std::size_t>(y * gfx_.Width() + x);
          if (z > depth_[idx]) {
            continue;
          }

          const double u = w1 * a.u + w2 * b.u + w3 * c.u;
          const double v = w1 * a.v + w2 * b.v + w3 * c.v;
          int tx = static_cast<int>(u * static_cast<double>(spr.width - 1));
          int ty = static_cast<int>(v * static_cast<double>(spr.height - 1));
          tx = std::max(0, std::min(spr.width - 1, tx));
          ty = std::max(0, std::min(spr.height - 1, ty));
          const auto& t = spr.texels[static_cast<std::size_t>(ty * spr.width + tx)];
          if (t.a == 0) {
            continue;
          }
          depth_[idx] = z;
          if (t.a == 255) {
            gfx_.PixelAtFast(x, y, static_cast<int>(t.r), static_cast<int>(t.g),
                             static_cast<int>(t.b));
          } else {
            // Translucent texels blend with current framebuffer color.
            const auto base_idx = static_cast<std::size_t>(y * gfx_.Width() + x);
            Pixel& out = gfx_.pixels[base_idx];
            const int a8 = static_cast<int>(t.a);
            out.r = (static_cast<int>(t.r) * a8 + out.r * (255 - a8)) / 255;
            out.g = (static_cast<int>(t.g) * a8 + out.g * (255 - a8)) / 255;
            out.b = (static_cast<int>(t.b) * a8 + out.b * (255 - a8)) / 255;
          }
        }
      }
    }

    GraphicsState& gfx_;
    Vec3 cam_{0.0, 0.0, -220.0};
    Vec3 rot_deg_{0.0, 0.0, 0.0};
    Vec3 trans_{0.0, 0.0, 0.0};
    Vec3 scale_{1.0, 1.0, 1.0};
    double fov_ = 300.0;
    double near_clip_ = 1.0;
    double far_clip_ = 10000.0;
    std::vector<double> depth_;
    bool depth_dirty_ = true;

    static int ClampColor(int v) { return std::max(0, std::min(255, v)); }

    void RequireGfx(const std::string& fn) const {
      if (!gfx_.IsOpen()) {
        throw std::runtime_error(fn +
                                 " requires gfx.open(...) or gfx.window(...) first");
      }
    }
  };

  class NetState {
   public:
    void Host(int port) {
      EnsureWinsock();
      OpenSocket();
      sockaddr_in addr{};
      addr.sin_family = AF_INET;
#ifdef _WIN32
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(static_cast<u_short>(port));
#else
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
      addr.sin_port = htons(static_cast<uint16_t>(port));
#endif
      if (bind(sock_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) !=
          0) {
        throw std::runtime_error("net.host bind failed");
      }
      is_host_ = true;
      open_ = true;
      has_remote_ = false;
      has_state_ = false;
    }

    void Join(const std::string& host, int port) {
      EnsureWinsock();
      OpenSocket();
      sockaddr_in addr{};
      addr.sin_family = AF_INET;
#ifdef _WIN32
      addr.sin_port = htons(static_cast<u_short>(port));
#else
      addr.sin_port = htons(static_cast<uint16_t>(port));
#endif
      int pton_res =
          inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
      if (pton_res != 1) {
        throw std::runtime_error("net.join invalid IPv4 address: " + host);
      }
      remote_addr_ = addr;
      has_remote_ = true;
      is_host_ = false;
      open_ = true;
      has_state_ = false;
    }

    int Poll() {
      if (!open_ || !SocketValid(sock_)) {
        return 0;
      }
      int count = 0;
      char buf[256];
      while (true) {
        sockaddr_in from{};
        SocketLen from_len = static_cast<SocketLen>(sizeof(from));
        int rc = recvfrom(sock_, buf, static_cast<int>(sizeof(buf) - 1), 0,
                          reinterpret_cast<sockaddr*>(&from), &from_len);
        if (rc < 0) {
#ifdef _WIN32
          int err = WSAGetLastError();
          if (err == WSAEWOULDBLOCK) {
            break;
          }
#else
          int err = errno;
          if (err == EWOULDBLOCK || err == EAGAIN) {
            break;
          }
#endif
          throw std::runtime_error("net.poll recvfrom failed");
        }
        if (rc == 0) {
          break;
        }
        buf[rc] = '\0';
        ParsePacket(std::string(buf));
        if (is_host_ && !has_remote_) {
          remote_addr_ = from;
          has_remote_ = true;
        }
        count += 1;
      }
      return count;
    }

    int SendPose(int x, int y, int z, int yaw, int pitch) {
      if (!open_ || !SocketValid(sock_) || !has_remote_) {
        return 0;
      }
      std::ostringstream out;
      out << "PYPPMP1 " << x << " " << y << " " << z << " " << yaw << " "
          << pitch;
      const std::string payload = out.str();
      int rc = sendto(sock_, payload.c_str(), static_cast<int>(payload.size()), 0,
                      reinterpret_cast<const sockaddr*>(&remote_addr_),
                      sizeof(remote_addr_));
      if (rc < 0) {
        return 0;
      }
      return 1;
    }

    int IsOpen() const { return open_ ? 1 : 0; }
    int HasRemote() const { return has_remote_ ? 1 : 0; }
    int HasState() const { return has_state_ ? 1 : 0; }
    int RemoteX() const { return remote_x_; }
    int RemoteY() const { return remote_y_; }
    int RemoteZ() const { return remote_z_; }
    int RemoteYaw() const { return remote_yaw_; }
    int RemotePitch() const { return remote_pitch_; }

    void Close() {
      CloseSocket();
      open_ = false;
      has_remote_ = false;
      has_state_ = false;
    }

    ~NetState() { Close(); }

   private:
#ifdef _WIN32
    using SocketHandle = SOCKET;
    using SocketLen = int;
    static constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
    using SocketHandle = int;
    using SocketLen = socklen_t;
    static constexpr SocketHandle kInvalidSocket = -1;
#endif

    static bool SocketValid(SocketHandle sock) {
#ifdef _WIN32
      return sock != INVALID_SOCKET;
#else
      return sock >= 0;
#endif
    }

#ifdef _WIN32
    void EnsureWinsock() {
      if (winsock_ready_) {
        return;
      }
      WSADATA wsa{};
      int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
      if (rc != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
      }
      winsock_ready_ = true;
    }
#else
    void EnsureWinsock() {}
#endif

    void OpenSocket() {
      Close();
      sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (!SocketValid(sock_)) {
        throw std::runtime_error("net socket creation failed");
      }
#ifdef _WIN32
      u_long non_blocking = 1;
      if (ioctlsocket(sock_, FIONBIO, &non_blocking) != 0) {
        closesocket(sock_);
        sock_ = kInvalidSocket;
        throw std::runtime_error("net socket non-blocking setup failed");
      }
#else
      int flags = fcntl(sock_, F_GETFL, 0);
      if (flags < 0 || fcntl(sock_, F_SETFL, flags | O_NONBLOCK) != 0) {
        close(sock_);
        sock_ = kInvalidSocket;
        throw std::runtime_error("net socket non-blocking setup failed");
      }
#endif
      open_ = true;
    }

    void CloseSocket() {
      if (!SocketValid(sock_)) {
        return;
      }
#ifdef _WIN32
      closesocket(sock_);
#else
      close(sock_);
#endif
      sock_ = kInvalidSocket;
    }

    void ParsePacket(const std::string& packet) {
      std::istringstream in(packet);
      std::string magic;
      int x = 0;
      int y = 0;
      int z = 0;
      int yaw = 0;
      int pitch = 0;
      in >> magic >> x >> y >> z >> yaw >> pitch;
      if (!in || magic != "PYPPMP1") {
        return;
      }
      remote_x_ = x;
      remote_y_ = y;
      remote_z_ = z;
      remote_yaw_ = yaw;
      remote_pitch_ = pitch;
      has_state_ = true;
    }

    bool open_ = false;
    bool is_host_ = false;
    bool has_remote_ = false;
    bool has_state_ = false;
    int remote_x_ = 0;
    int remote_y_ = 0;
    int remote_z_ = 0;
    int remote_yaw_ = 0;
    int remote_pitch_ = 0;
#if defined(_WIN32)
    bool winsock_ready_ = false;
#endif
    SocketHandle sock_ = kInvalidSocket;
    sockaddr_in remote_addr_{};
  };

  Value Pop() {
    if (stack_.empty()) {
      throw std::runtime_error("Stack underflow");
    }
    Value v = stack_.back();
    stack_.pop_back();
    return v;
  }

  void RunArithmetic(const std::string& op) {
    int rhs = ValueAsInt(Pop(), op);
    int lhs = ValueAsInt(Pop(), op);
    if (op == "ADD") {
      stack_.push_back(lhs + rhs);
    } else if (op == "SUB") {
      stack_.push_back(lhs - rhs);
    } else if (op == "MUL") {
      stack_.push_back(lhs * rhs);
    } else {
      if (rhs == 0) {
        throw std::runtime_error("Division by zero");
      }
      stack_.push_back(lhs / rhs);
    }
  }

  void RunComparison(const std::string& op) {
    int rhs = ValueAsInt(Pop(), op);
    int lhs = ValueAsInt(Pop(), op);
    if (op == "CMP_EQ") {
      stack_.push_back(lhs == rhs ? 1 : 0);
    } else if (op == "CMP_NE") {
      stack_.push_back(lhs != rhs ? 1 : 0);
    } else if (op == "CMP_LT") {
      stack_.push_back(lhs < rhs ? 1 : 0);
    } else if (op == "CMP_LE") {
      stack_.push_back(lhs <= rhs ? 1 : 0);
    } else if (op == "CMP_GT") {
      stack_.push_back(lhs > rhs ? 1 : 0);
    } else {
      stack_.push_back(lhs >= rhs ? 1 : 0);
    }
  }

  std::vector<Value> PopArgs(int argc) {
    if (argc < 0 || static_cast<std::size_t>(argc) > stack_.size()) {
      throw std::runtime_error("Invalid argument count on stack");
    }
    std::vector<Value> args(static_cast<std::size_t>(argc));
    for (int i = argc - 1; i >= 0; --i) {
      args[static_cast<std::size_t>(i)] = Pop();
    }
    return args;
  }

  void RunCall(const std::string& name, int argc) {
    std::vector<Value> args = PopArgs(argc);
    if (name == "print") {
      for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
          std::cout << " ";
        }
        std::cout << ValueToString(args[i]);
      }
      std::cout << "\n";
      stack_.push_back(0);
      return;
    }
    if (name == "torch.seed") {
      ExpectArgc(name, argc, 1);
      torch_seed_ = static_cast<std::uint32_t>(ValueAsInt(args[0], name));
      torch_rng_.seed(torch_seed_);
      stack_.push_back(0);
      return;
    }
    if (name == "torch.rand_int") {
      ExpectArgc(name, argc, 2);
      int lo = ValueAsInt(args[0], name);
      int hi = ValueAsInt(args[1], name);
      if (lo > hi) {
        std::swap(lo, hi);
      }
      std::uniform_int_distribution<int> dist(lo, hi);
      stack_.push_back(dist(torch_rng_));
      return;
    }
    if (name == "torch.rand_norm") {
      ExpectArgc(name, argc, 1);
      const int scale = ValueAsInt(args[0], name);
      std::normal_distribution<double> dist(0.0, 1.0);
      stack_.push_back(static_cast<int>(std::round(dist(torch_rng_) *
                                                   static_cast<double>(scale))));
      return;
    }
    if (name == "torch.relu") {
      ExpectArgc(name, argc, 1);
      int x = ValueAsInt(args[0], name);
      stack_.push_back(x > 0 ? x : 0);
      return;
    }
    if (name == "torch.leaky_relu") {
      ExpectArgc(name, argc, 2);
      int x = ValueAsInt(args[0], name);
      int alpha_ppm = ValueAsInt(args[1], name);
      if (x >= 0) {
        stack_.push_back(x);
      } else {
        stack_.push_back(static_cast<int>((static_cast<long long>(x) *
                                           static_cast<long long>(alpha_ppm)) /
                                          1000000LL));
      }
      return;
    }
    if (name == "torch.sigmoid") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(TorchSigmoidPpm(ValueAsInt(args[0], name)));
      return;
    }
    if (name == "torch.tanh") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(TorchTanhPpm(ValueAsInt(args[0], name)));
      return;
    }
    if (name == "torch.dot3") {
      ExpectArgc(name, argc, 6);
      long long v =
          static_cast<long long>(ValueAsInt(args[0], name)) *
              static_cast<long long>(ValueAsInt(args[3], name)) +
          static_cast<long long>(ValueAsInt(args[1], name)) *
              static_cast<long long>(ValueAsInt(args[4], name)) +
          static_cast<long long>(ValueAsInt(args[2], name)) *
              static_cast<long long>(ValueAsInt(args[5], name));
      stack_.push_back(static_cast<int>(v));
      return;
    }
    if (name == "torch.mse") {
      ExpectArgc(name, argc, 2);
      long long d = static_cast<long long>(ValueAsInt(args[0], name)) -
                    static_cast<long long>(ValueAsInt(args[1], name));
      long long v = d * d;
      if (v > static_cast<long long>(std::numeric_limits<int>::max())) {
        v = static_cast<long long>(std::numeric_limits<int>::max());
      }
      stack_.push_back(static_cast<int>(v));
      return;
    }
    if (name == "torch.lerp") {
      ExpectArgc(name, argc, 3);
      int a = ValueAsInt(args[0], name);
      int b = ValueAsInt(args[1], name);
      int t_ppm = ValueAsInt(args[2], name);
      if (t_ppm < 0) t_ppm = 0;
      if (t_ppm > 1000000) t_ppm = 1000000;
      long long out =
          static_cast<long long>(a) +
          (static_cast<long long>(b - a) * static_cast<long long>(t_ppm)) /
              1000000LL;
      stack_.push_back(static_cast<int>(out));
      return;
    }
    if (name == "torch.step") {
      ExpectArgc(name, argc, 3);
      int param = ValueAsInt(args[0], name);
      int grad = ValueAsInt(args[1], name);
      int lr_ppm = ValueAsInt(args[2], name);
      long long delta =
          (static_cast<long long>(grad) * static_cast<long long>(lr_ppm)) /
          1000000LL;
      stack_.push_back(static_cast<int>(static_cast<long long>(param) - delta));
      return;
    }
    if (name == "math.array" || name == "numpy.array") {
      stack_.push_back(MakeListFromArgs(args));
      return;
    }
    if (name == "math.len" || name == "numpy.len") {
      ExpectArgc(name, argc, 1);
      ListPtr list = ValueAsListPtr(args[0], name);
      stack_.push_back(static_cast<int>(list->items.size()));
      return;
    }
    if (name == "math.get" || name == "numpy.get") {
      ExpectArgc(name, argc, 2);
      ListPtr list = ValueAsListPtr(args[0], name);
      int idx = NormalizeIndex(ValueAsInt(args[1], name),
                               static_cast<int>(list->items.size()), name);
      stack_.push_back(list->items[static_cast<std::size_t>(idx)]);
      return;
    }
    if (name == "math.set" || name == "numpy.set") {
      ExpectArgc(name, argc, 3);
      ListPtr list = ValueAsListPtr(args[0], name);
      int idx = NormalizeIndex(ValueAsInt(args[1], name),
                               static_cast<int>(list->items.size()), name);
      list->items[static_cast<std::size_t>(idx)] = args[2];
      stack_.push_back(0);
      return;
    }
    if (name == "math.push" || name == "numpy.push") {
      ExpectArgc(name, argc, 2);
      ListPtr list = ValueAsListPtr(args[0], name);
      list->items.push_back(args[1]);
      stack_.push_back(static_cast<int>(list->items.size()));
      return;
    }
    if (name == "math.pop" || name == "numpy.pop") {
      ExpectArgc(name, argc, 1);
      ListPtr list = ValueAsListPtr(args[0], name);
      if (list->items.empty()) {
        throw std::runtime_error(name + ": pop from empty list");
      }
      Value v = list->items.back();
      list->items.pop_back();
      stack_.push_back(v);
      return;
    }
    if (name == "math.zeros" || name == "numpy.zeros") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(MakeFilledIntList(ValueAsInt(args[0], name), 0, name));
      return;
    }
    if (name == "math.ones" || name == "numpy.ones") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(MakeFilledIntList(ValueAsInt(args[0], name), 1, name));
      return;
    }
    if (name == "math.arange" || name == "numpy.arange") {
      int start = 0;
      int stop = 0;
      int step = 1;
      if (argc == 1) {
        stop = ValueAsInt(args[0], name);
      } else if (argc == 2) {
        start = ValueAsInt(args[0], name);
        stop = ValueAsInt(args[1], name);
      } else if (argc == 3) {
        start = ValueAsInt(args[0], name);
        stop = ValueAsInt(args[1], name);
        step = ValueAsInt(args[2], name);
      } else {
        throw std::runtime_error(name + " expects 1, 2, or 3 args");
      }
      if (step == 0) {
        throw std::runtime_error(name + ": step must not be 0");
      }
      ListPtr out = std::make_shared<List>();
      if (step > 0) {
        for (int v = start; v < stop; v += step) {
          out->items.push_back(v);
        }
      } else {
        for (int v = start; v > stop; v += step) {
          out->items.push_back(v);
        }
      }
      stack_.push_back(out);
      return;
    }
    if (name == "math.linspace" || name == "numpy.linspace") {
      ExpectArgc(name, argc, 3);
      int start = ValueAsInt(args[0], name);
      int stop = ValueAsInt(args[1], name);
      int count = ValueAsInt(args[2], name);
      if (count <= 0) {
        throw std::runtime_error(name + ": count must be > 0");
      }
      ListPtr out = std::make_shared<List>();
      out->items.reserve(static_cast<std::size_t>(count));
      if (count == 1) {
        out->items.push_back(start);
      } else {
        const double dstart = static_cast<double>(start);
        const double dstop = static_cast<double>(stop);
        const double n = static_cast<double>(count - 1);
        for (int i = 0; i < count; ++i) {
          double t = static_cast<double>(i) / n;
          out->items.push_back(
              static_cast<int>(std::round(dstart + (dstop - dstart) * t)));
        }
      }
      stack_.push_back(out);
      return;
    }
    if (name == "math.sum" || name == "numpy.sum") {
      ExpectArgc(name, argc, 1);
      ListPtr list = ValueAsListPtr(args[0], name);
      long long acc = 0;
      for (const Value& v : list->items) {
        acc += static_cast<long long>(ValueAsInt(v, name));
      }
      if (acc > static_cast<long long>(std::numeric_limits<int>::max())) {
        acc = static_cast<long long>(std::numeric_limits<int>::max());
      }
      if (acc < static_cast<long long>(std::numeric_limits<int>::min())) {
        acc = static_cast<long long>(std::numeric_limits<int>::min());
      }
      stack_.push_back(static_cast<int>(acc));
      return;
    }
    if (name == "math.mean" || name == "numpy.mean") {
      ExpectArgc(name, argc, 1);
      ListPtr list = ValueAsListPtr(args[0], name);
      if (list->items.empty()) {
        throw std::runtime_error(name + ": empty list");
      }
      long long acc = 0;
      for (const Value& v : list->items) {
        acc += static_cast<long long>(ValueAsInt(v, name));
      }
      stack_.push_back(static_cast<int>(
          std::round(static_cast<double>(acc) /
                     static_cast<double>(list->items.size()))));
      return;
    }
    if (name == "math.min" || name == "numpy.min") {
      ExpectArgc(name, argc, 1);
      ListPtr list = ValueAsListPtr(args[0], name);
      if (list->items.empty()) {
        throw std::runtime_error(name + ": empty list");
      }
      int best = ValueAsInt(list->items[0], name);
      for (std::size_t i = 1; i < list->items.size(); ++i) {
        best = std::min(best, ValueAsInt(list->items[i], name));
      }
      stack_.push_back(best);
      return;
    }
    if (name == "math.max" || name == "numpy.max") {
      ExpectArgc(name, argc, 1);
      ListPtr list = ValueAsListPtr(args[0], name);
      if (list->items.empty()) {
        throw std::runtime_error(name + ": empty list");
      }
      int best = ValueAsInt(list->items[0], name);
      for (std::size_t i = 1; i < list->items.size(); ++i) {
        best = std::max(best, ValueAsInt(list->items[i], name));
      }
      stack_.push_back(best);
      return;
    }
    if (name == "math.dot" || name == "numpy.dot") {
      ExpectArgc(name, argc, 2);
      ListPtr a = ValueAsListPtr(args[0], name);
      ListPtr b = ValueAsListPtr(args[1], name);
      if (a->items.size() != b->items.size()) {
        throw std::runtime_error(name + ": list sizes must match");
      }
      long long acc = 0;
      for (std::size_t i = 0; i < a->items.size(); ++i) {
        acc += static_cast<long long>(ValueAsInt(a->items[i], name)) *
               static_cast<long long>(ValueAsInt(b->items[i], name));
      }
      if (acc > static_cast<long long>(std::numeric_limits<int>::max())) {
        acc = static_cast<long long>(std::numeric_limits<int>::max());
      }
      if (acc < static_cast<long long>(std::numeric_limits<int>::min())) {
        acc = static_cast<long long>(std::numeric_limits<int>::min());
      }
      stack_.push_back(static_cast<int>(acc));
      return;
    }
    if (name == "math.add" || name == "numpy.add") {
      ExpectArgc(name, argc, 2);
      stack_.push_back(ElementwiseBinary(args[0], args[1], name, '+'));
      return;
    }
    if (name == "math.sub" || name == "numpy.sub") {
      ExpectArgc(name, argc, 2);
      stack_.push_back(ElementwiseBinary(args[0], args[1], name, '-'));
      return;
    }
    if (name == "math.mul" || name == "numpy.mul") {
      ExpectArgc(name, argc, 2);
      stack_.push_back(ElementwiseBinary(args[0], args[1], name, '*'));
      return;
    }
    if (name == "math.div" || name == "numpy.div") {
      ExpectArgc(name, argc, 2);
      stack_.push_back(ElementwiseBinary(args[0], args[1], name, '/'));
      return;
    }
    if (name == "math.clip" || name == "numpy.clip") {
      ExpectArgc(name, argc, 3);
      ListPtr list = ValueAsListPtr(args[0], name);
      int lo = ValueAsInt(args[1], name);
      int hi = ValueAsInt(args[2], name);
      if (lo > hi) {
        std::swap(lo, hi);
      }
      ListPtr out = std::make_shared<List>();
      out->items.reserve(list->items.size());
      for (const Value& v : list->items) {
        int x = ValueAsInt(v, name);
        if (x < lo) x = lo;
        if (x > hi) x = hi;
        out->items.push_back(x);
      }
      stack_.push_back(out);
      return;
    }
    if (name == "math.abs" || name == "numpy.abs") {
      ExpectArgc(name, argc, 1);
      if (std::holds_alternative<ListPtr>(args[0])) {
        ListPtr list = ValueAsListPtr(args[0], name);
        ListPtr out = std::make_shared<List>();
        out->items.reserve(list->items.size());
        for (const Value& v : list->items) {
          out->items.push_back(std::abs(ValueAsInt(v, name)));
        }
        stack_.push_back(out);
      } else {
        stack_.push_back(std::abs(ValueAsInt(args[0], name)));
      }
      return;
    }
    if (name == "random.seed") {
      ExpectArgc(name, argc, 1);
      const int seed = ValueAsInt(args[0], name);
      random_seed_ = static_cast<std::uint32_t>(seed);
      rng_.seed(random_seed_);
      stack_.push_back(0);
      return;
    }
    if (name == "random.randint") {
      ExpectArgc(name, argc, 2);
      int lo = ValueAsInt(args[0], name);
      int hi = ValueAsInt(args[1], name);
      if (lo > hi) {
        std::swap(lo, hi);
      }
      std::uniform_int_distribution<int> dist(lo, hi);
      stack_.push_back(dist(rng_));
      return;
    }
    if (name == "random.randrange") {
      ExpectArgc(name, argc, 2);
      int start = ValueAsInt(args[0], name);
      int stop = ValueAsInt(args[1], name);
      if (stop <= start) {
        throw std::runtime_error("random.randrange expects stop > start");
      }
      std::uniform_int_distribution<int> dist(start, stop - 1);
      stack_.push_back(dist(rng_));
      return;
    }
    if (name == "random.random") {
      ExpectArgc(name, argc, 0);
      // py++ currently has integer values only, so this returns a fixed-point
      // random value in [0, 1_000_000].
      std::uniform_int_distribution<int> dist(0, 1000000);
      stack_.push_back(dist(rng_));
      return;
    }
    if (name == "random.chance") {
      ExpectArgc(name, argc, 1);
      int pct = ValueAsInt(args[0], name);
      if (pct <= 0) {
        stack_.push_back(0);
        return;
      }
      if (pct >= 100) {
        stack_.push_back(1);
        return;
      }
      std::uniform_int_distribution<int> dist(0, 99);
      stack_.push_back(dist(rng_) < pct ? 1 : 0);
      return;
    }
    if (name == "noise.seed") {
      ExpectArgc(name, argc, 1);
      noise_seed_ = static_cast<std::uint32_t>(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "noise.value2") {
      ExpectArgc(name, argc, 2);
      stack_.push_back(NoiseValue2(ValueAsInt(args[0], name),
                                   ValueAsInt(args[1], name)));
      return;
    }
    if (name == "noise.value3") {
      ExpectArgc(name, argc, 3);
      stack_.push_back(NoiseValue3(ValueAsInt(args[0], name),
                                   ValueAsInt(args[1], name),
                                   ValueAsInt(args[2], name)));
      return;
    }
    if (name == "noise.smooth2") {
      ExpectArgc(name, argc, 3);
      const int scale = ValueAsInt(args[2], name);
      if (scale <= 0) {
        throw std::runtime_error("noise.smooth2 expects scale > 0");
      }
      stack_.push_back(NoiseSmooth2(ValueAsInt(args[0], name),
                                    ValueAsInt(args[1], name), scale));
      return;
    }
    if (name == "noise.fractal2") {
      ExpectArgc(name, argc, 5);
      const int x = ValueAsInt(args[0], name);
      const int y = ValueAsInt(args[1], name);
      const int scale = ValueAsInt(args[2], name);
      const int octaves = ValueAsInt(args[3], name);
      const int persistence_pct = ValueAsInt(args[4], name);
      if (scale <= 0) {
        throw std::runtime_error("noise.fractal2 expects scale > 0");
      }
      if (octaves <= 0) {
        throw std::runtime_error("noise.fractal2 expects octaves > 0");
      }
      if (persistence_pct <= 0 || persistence_pct > 100) {
        throw std::runtime_error(
            "noise.fractal2 expects persistence in range 1..100");
      }
      stack_.push_back(
          NoiseFractal2(x, y, scale, octaves, persistence_pct));
      return;
    }
    if (name == "net.host") {
      ExpectArgc(name, argc, 1);
      net_.Host(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "net.join") {
      ExpectArgc(name, argc, 2);
      if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("net.join expects IPv4 string and port");
      }
      net_.Join(std::get<std::string>(args[0]), ValueAsInt(args[1], name));
      stack_.push_back(0);
      return;
    }
    if (name == "net.poll") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.Poll());
      return;
    }
    if (name == "net.send_pose") {
      ExpectArgc(name, argc, 5);
      stack_.push_back(
          net_.SendPose(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                        ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                        ValueAsInt(args[4], name)));
      return;
    }
    if (name == "net.open") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.IsOpen());
      return;
    }
    if (name == "net.has_remote") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.HasRemote());
      return;
    }
    if (name == "net.has_state") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.HasState());
      return;
    }
    if (name == "net.remote_x") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.RemoteX());
      return;
    }
    if (name == "net.remote_y") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.RemoteY());
      return;
    }
    if (name == "net.remote_z") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.RemoteZ());
      return;
    }
    if (name == "net.remote_yaw") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.RemoteYaw());
      return;
    }
    if (name == "net.remote_pitch") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(net_.RemotePitch());
      return;
    }
    if (name == "net.close") {
      ExpectArgc(name, argc, 0);
      net_.Close();
      stack_.push_back(0);
      return;
    }

    if (name == "gfx.open") {
      ExpectArgc(name, argc, 2);
      gfx_.Open(ValueAsInt(args[0], name), ValueAsInt(args[1], name));
      gx3d_.OnFrameReset();
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.clear") {
      ExpectArgc(name, argc, 3);
      gfx_.Clear(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                 ValueAsInt(args[2], name));
      gx3d_.OnFrameReset();
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.pixel") {
      ExpectArgc(name, argc, 5);
      gfx_.PixelAt(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                   ValueAsInt(args[4], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.save") {
      ExpectArgc(name, argc, 1);
      if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("gfx.save expects a path string");
      }
      gfx_.Save(std::get<std::string>(args[0]));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.save_frame") {
      ExpectArgc(name, argc, 2);
      if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("gfx.save_frame expects (string, int)");
      }
      gfx_.SaveFrame(std::get<std::string>(args[0]), ValueAsInt(args[1], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.line") {
      ExpectArgc(name, argc, 7);
      gfx_.Line(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.rect") {
      ExpectArgc(name, argc, 7);
      gfx_.Rect(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.rect_outline") {
      ExpectArgc(name, argc, 7);
      gfx_.RectOutline(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                       ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                       ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                       ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.circle") {
      ExpectArgc(name, argc, 6);
      gfx_.Circle(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                  ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                  ValueAsInt(args[4], name), ValueAsInt(args[5], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.width") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.Width());
      return;
    }
    if (name == "gfx.height") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.Height());
      return;
    }
    if (name == "gfx.window") {
      ExpectArgc(name, argc, 3);
      if (!std::holds_alternative<std::string>(args[2])) {
        throw std::runtime_error("gfx.window expects title string as third argument");
      }
      gfx_.OpenWindow(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                      std::get<std::string>(args[2]));
      gx3d_.OnFrameReset();
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.window_ratio") {
      ExpectArgc(name, argc, 5);
      if (!std::holds_alternative<std::string>(args[4])) {
        throw std::runtime_error(
            "gfx.window_ratio expects title string as fifth argument");
      }
      gfx_.OpenWindowRatio(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                           ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                           std::get<std::string>(args[4]));
      gx3d_.OnFrameReset();
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.keep_aspect") {
      ExpectArgc(name, argc, 1);
      gfx_.SetKeepAspect(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.refresh_rate") {
      ExpectArgc(name, argc, 1);
      gfx_.SetRefreshRate(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.poll") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.PollEvents());
      return;
    }
    if (name == "gfx.present") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.Present());
      gx3d_.OnFrameReset();
      return;
    }
    if (name == "gfx.sync") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.SyncFrame());
      return;
    }
    if (name == "gfx.key_down") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(gfx_.KeyDown(ValueAsInt(args[0], name)));
      return;
    }
    if (name == "gfx.mouse_x") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.MouseX());
      return;
    }
    if (name == "gfx.mouse_y") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.MouseY());
      return;
    }
    if (name == "gfx.mouse_down") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(gfx_.MouseDown(ValueAsInt(args[0], name)));
      return;
    }
    if (name == "gfx.mouse_dx") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.ConsumeMouseDX());
      return;
    }
    if (name == "gfx.mouse_dy") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.ConsumeMouseDY());
      return;
    }
    if (name == "gfx.mouse_lock") {
      ExpectArgc(name, argc, 1);
      gfx_.SetMouseLock(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.mouse_show") {
      ExpectArgc(name, argc, 1);
      gfx_.SetMouseVisible(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.button") {
      ExpectArgc(name, argc, 4);
      stack_.push_back(gfx_.Button(ValueAsInt(args[0], name),
                                   ValueAsInt(args[1], name),
                                   ValueAsInt(args[2], name),
                                   ValueAsInt(args[3], name)));
      return;
    }
    if (name == "gfx.closed") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.IsClosed());
      return;
    }
    if (name == "gfx.close") {
      ExpectArgc(name, argc, 0);
      gfx_.CloseWindow();
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.load_sprite") {
      ExpectArgc(name, argc, 1);
      if (!std::holds_alternative<std::string>(args[0])) {
        throw std::runtime_error("gfx.load_sprite expects path string");
      }
      int id = gfx_.LoadSprite(std::get<std::string>(args[0]));
      stack_.push_back(id);
      return;
    }
    if (name == "gfx.draw_sprite") {
      ExpectArgc(name, argc, 3);
      gfx_.DrawSprite(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                      ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.draw_sprite_scaled") {
      ExpectArgc(name, argc, 5);
      gfx_.DrawSpriteScaled(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                            ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                            ValueAsInt(args[4], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.shader_set") {
      ExpectArgc(name, argc, 4);
      gfx_.ShaderSet(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                     ValueAsInt(args[2], name), ValueAsInt(args[3], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.shader_clear") {
      ExpectArgc(name, argc, 0);
      gfx_.ShaderClear();
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.shader_create") {
      ExpectArgc(name, argc, 0);
      stack_.push_back(gfx_.ShaderCreate());
      return;
    }
    if (name == "gfx.shader_program_clear") {
      ExpectArgc(name, argc, 1);
      gfx_.ShaderProgramClear(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.shader_add") {
      ExpectArgc(name, argc, 5);
      gfx_.ShaderAdd(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                     ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                     ValueAsInt(args[4], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.shader_program_len") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(gfx_.ShaderProgramLen(ValueAsInt(args[0], name)));
      return;
    }
    if (name == "gfx.shader_use_program") {
      ExpectArgc(name, argc, 1);
      gfx_.ShaderUseProgram(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.anim_register") {
      ExpectArgc(name, argc, 4);
      stack_.push_back(
          gfx_.AnimRegister(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                            ValueAsInt(args[2], name), ValueAsInt(args[3], name)));
      return;
    }
    if (name == "gfx.anim_frame") {
      ExpectArgc(name, argc, 2);
      stack_.push_back(
          gfx_.AnimFrame(ValueAsInt(args[0], name), ValueAsInt(args[1], name)));
      return;
    }
    if (name == "gfx.anim_length") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(gfx_.AnimLength(ValueAsInt(args[0], name)));
      return;
    }
    if (name == "gfx.anim_draw") {
      ExpectArgc(name, argc, 4);
      gfx_.AnimDraw(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                    ValueAsInt(args[2], name), ValueAsInt(args[3], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.anim_draw_scaled") {
      ExpectArgc(name, argc, 6);
      gfx_.AnimDrawScaled(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                          ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                          ValueAsInt(args[4], name), ValueAsInt(args[5], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.text") {
      ExpectArgc(name, argc, 6);
      std::string text_value;
      if (std::holds_alternative<std::string>(args[2])) {
        text_value = std::get<std::string>(args[2]);
      } else if (std::holds_alternative<int>(args[2])) {
        text_value = std::to_string(std::get<int>(args[2]));
      } else {
        throw std::runtime_error("gfx.text expects text as string or int");
      }
      gfx_.Text(ValueAsInt(args[0], name), ValueAsInt(args[1], name), text_value,
                ValueAsInt(args[3], name),
                ValueAsInt(args[4], name), ValueAsInt(args[5], name));
      stack_.push_back(0);
      return;
    }
    if (name == "time.sleep_ms") {
      ExpectArgc(name, argc, 1);
      int ms = ValueAsInt(args[0], name);
      if (ms < 0) {
        ms = 0;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(ms));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.reset") {
      ExpectArgc(name, argc, 0);
      gx3d_.Reset();
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.camera") {
      ExpectArgc(name, argc, 3);
      gx3d_.Camera(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.camera_move") {
      ExpectArgc(name, argc, 3);
      gx3d_.CameraMove(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                       ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.rotate") {
      ExpectArgc(name, argc, 3);
      gx3d_.Rotate(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.rotate_add") {
      ExpectArgc(name, argc, 3);
      gx3d_.RotateAdd(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                      ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.translate") {
      ExpectArgc(name, argc, 3);
      gx3d_.Translate(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                      ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.scale") {
      ExpectArgc(name, argc, 3);
      gx3d_.Scale(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                  ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.scale_uniform") {
      ExpectArgc(name, argc, 1);
      gx3d_.ScaleUniform(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.fov") {
      ExpectArgc(name, argc, 1);
      gx3d_.Fov(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.clip") {
      ExpectArgc(name, argc, 2);
      gx3d_.Clip(ValueAsInt(args[0], name), ValueAsInt(args[1], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.point") {
      ExpectArgc(name, argc, 6);
      gx3d_.Point(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                  ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                  ValueAsInt(args[4], name), ValueAsInt(args[5], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.line") {
      ExpectArgc(name, argc, 9);
      gx3d_.Line3d(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                   ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                   ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                   ValueAsInt(args[8], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.cube") {
      ExpectArgc(name, argc, 7);
      gx3d_.Cube(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                 ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                 ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                 ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.cube_solid") {
      ExpectArgc(name, argc, 7);
      gx3d_.CubeSolid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                      ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                      ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                      ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.triangle") {
      ExpectArgc(name, argc, 12);
      gx3d_.Triangle(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                     ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                     ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                     ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                     ValueAsInt(args[8], name), ValueAsInt(args[9], name),
                     ValueAsInt(args[10], name), ValueAsInt(args[11], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.triangle_solid") {
      ExpectArgc(name, argc, 12);
      gx3d_.TriangleSolid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                          ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                          ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                          ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                          ValueAsInt(args[8], name), ValueAsInt(args[9], name),
                          ValueAsInt(args[10], name), ValueAsInt(args[11], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.quad") {
      ExpectArgc(name, argc, 15);
      gx3d_.Quad(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                 ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                 ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                 ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                 ValueAsInt(args[8], name), ValueAsInt(args[9], name),
                 ValueAsInt(args[10], name), ValueAsInt(args[11], name),
                 ValueAsInt(args[12], name), ValueAsInt(args[13], name),
                 ValueAsInt(args[14], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.quad_solid") {
      ExpectArgc(name, argc, 15);
      gx3d_.QuadSolid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                      ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                      ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                      ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                      ValueAsInt(args[8], name), ValueAsInt(args[9], name),
                      ValueAsInt(args[10], name), ValueAsInt(args[11], name),
                      ValueAsInt(args[12], name), ValueAsInt(args[13], name),
                      ValueAsInt(args[14], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.pyramid") {
      ExpectArgc(name, argc, 7);
      gx3d_.Pyramid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                    ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                    ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                    ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.pyramid_solid") {
      ExpectArgc(name, argc, 7);
      gx3d_.PyramidSolid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                         ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                         ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                         ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.cuboid") {
      ExpectArgc(name, argc, 9);
      gx3d_.Cuboid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                   ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                   ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                   ValueAsInt(args[8], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.cuboid_solid") {
      ExpectArgc(name, argc, 9);
      gx3d_.CuboidSolid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                        ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                        ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                        ValueAsInt(args[6], name), ValueAsInt(args[7], name),
                        ValueAsInt(args[8], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.cube_sprite") {
      ExpectArgc(name, argc, 5);
      gx3d_.CubeSprite(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                       ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                       ValueAsInt(args[4], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.cuboid_sprite") {
      ExpectArgc(name, argc, 7);
      gx3d_.CuboidSprite(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                         ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                         ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                         ValueAsInt(args[6], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.sphere") {
      ExpectArgc(name, argc, 8);
      gx3d_.Sphere(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name), ValueAsInt(args[3], name),
                   ValueAsInt(args[4], name), ValueAsInt(args[5], name),
                   ValueAsInt(args[6], name), ValueAsInt(args[7], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.axis") {
      ExpectArgc(name, argc, 1);
      gx3d_.Axis(ValueAsInt(args[0], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.grid") {
      ExpectArgc(name, argc, 3);
      gx3d_.Grid(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                 ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }

    throw std::runtime_error("Unknown function: " + name);
  }

  void RunImport(const std::string& module_name, const std::string& alias) {
    std::string module_file = module_name;
    std::replace(module_file.begin(), module_file.end(), '.', '/');
    module_file += ".pypp";
    std::filesystem::path module_rel = module_file;
    std::filesystem::path candidate = module_base_ / module_rel;
    if (!std::filesystem::exists(candidate)) {
      throw std::runtime_error("Import not found: " + candidate.string());
    }

    VM module_vm(candidate.parent_path());
    std::vector<Instruction> module_code = CompileSource(candidate);
    module_vm.Execute(module_code);
    ObjectPtr module_obj = std::make_shared<Object>();
    for (const auto& [name, value] : module_vm.Globals()) {
      module_obj->fields[name] = value;
    }
    vars_[alias] = module_obj;
  }

  static std::uint32_t HashU32(std::uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
  }

  static double Fade(double t) {
    return t * t * (3.0 - 2.0 * t);
  }

  static int ClampByte(int v) { return std::max(0, std::min(255, v)); }

  static int FloorDiv(int a, int b) {
    int q = a / b;
    int r = a % b;
    if (r != 0 && ((r > 0) != (b > 0))) {
      q -= 1;
    }
    return q;
  }

  static int PosMod(int a, int b) {
    int m = a % b;
    if (m < 0) {
      m += std::abs(b);
    }
    return m;
  }

  int NoiseValue2(int x, int y) const {
    std::uint32_t h = HashU32(static_cast<std::uint32_t>(x) * 0x9E3779B9U ^
                              static_cast<std::uint32_t>(y) * 0x85EBCA6BU ^
                              noise_seed_);
    return static_cast<int>(h & 255U);
  }

  int NoiseValue3(int x, int y, int z) const {
    std::uint32_t h = HashU32(static_cast<std::uint32_t>(x) * 0x9E3779B9U ^
                              static_cast<std::uint32_t>(y) * 0x85EBCA6BU ^
                              static_cast<std::uint32_t>(z) * 0xC2B2AE35U ^
                              noise_seed_);
    return static_cast<int>(h & 255U);
  }

  int NoiseSmooth2(int x, int y, int scale) const {
    const int cell_x = FloorDiv(x, scale);
    const int cell_y = FloorDiv(y, scale);
    const int frac_x = PosMod(x, scale);
    const int frac_y = PosMod(y, scale);

    const double tx = static_cast<double>(frac_x) / static_cast<double>(scale);
    const double ty = static_cast<double>(frac_y) / static_cast<double>(scale);
    const double ux = Fade(tx);
    const double uy = Fade(ty);

    const double v00 = static_cast<double>(NoiseValue2(cell_x, cell_y));
    const double v10 = static_cast<double>(NoiseValue2(cell_x + 1, cell_y));
    const double v01 = static_cast<double>(NoiseValue2(cell_x, cell_y + 1));
    const double v11 = static_cast<double>(NoiseValue2(cell_x + 1, cell_y + 1));
    const double a = v00 * (1.0 - ux) + v10 * ux;
    const double b = v01 * (1.0 - ux) + v11 * ux;
    return ClampByte(static_cast<int>(std::round(a * (1.0 - uy) + b * uy)));
  }

  int NoiseFractal2(int x, int y, int base_scale, int octaves,
                    int persistence_pct) const {
    double amp = 1.0;
    double sum = 0.0;
    double norm = 0.0;
    int scale = base_scale;
    for (int i = 0; i < octaves; ++i) {
      if (scale <= 0) {
        break;
      }
      sum += static_cast<double>(NoiseSmooth2(x, y, scale)) * amp;
      norm += amp;
      scale = std::max(1, scale / 2);
      amp *= static_cast<double>(persistence_pct) / 100.0;
    }
    if (norm <= 0.0) {
      return 0;
    }
    return ClampByte(static_cast<int>(std::round(sum / norm)));
  }

  static ListPtr ValueAsListPtr(const Value& value, const std::string& context) {
    if (!std::holds_alternative<ListPtr>(value) || !std::get<ListPtr>(value)) {
      throw std::runtime_error(context + ": expected list");
    }
    return std::get<ListPtr>(value);
  }

  static int NormalizeIndex(int idx, int n, const std::string& context) {
    int out = idx;
    if (out < 0) {
      out += n;
    }
    if (out < 0 || out >= n) {
      throw std::runtime_error(context + ": index out of range");
    }
    return out;
  }

  static ListPtr MakeListFromArgs(const std::vector<Value>& args) {
    ListPtr list = std::make_shared<List>();
    list->items = args;
    return list;
  }

  static ListPtr MakeFilledIntList(int count, int value, const std::string& context) {
    if (count < 0) {
      throw std::runtime_error(context + ": count must be >= 0");
    }
    ListPtr list = std::make_shared<List>();
    list->items.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
      list->items.push_back(value);
    }
    return list;
  }

  static ListPtr ElementwiseBinary(const Value& a, const Value& b,
                                   const std::string& context, char op) {
    const bool a_is_list = std::holds_alternative<ListPtr>(a);
    const bool b_is_list = std::holds_alternative<ListPtr>(b);

    auto apply_op = [&](int lhs, int rhs) -> int {
      switch (op) {
        case '+':
          return lhs + rhs;
        case '-':
          return lhs - rhs;
        case '*':
          return lhs * rhs;
        case '/':
          if (rhs == 0) {
            throw std::runtime_error(context + ": division by zero");
          }
          return lhs / rhs;
        default:
          throw std::runtime_error(context + ": unknown op");
      }
    };

    ListPtr out = std::make_shared<List>();
    if (!a_is_list && !b_is_list) {
      out->items.push_back(apply_op(ValueAsInt(a, context), ValueAsInt(b, context)));
      return out;
    }
    if (a_is_list && b_is_list) {
      ListPtr la = ValueAsListPtr(a, context);
      ListPtr lb = ValueAsListPtr(b, context);
      if (la->items.size() != lb->items.size()) {
        throw std::runtime_error(context + ": list sizes must match");
      }
      out->items.reserve(la->items.size());
      for (std::size_t i = 0; i < la->items.size(); ++i) {
        out->items.push_back(
            apply_op(ValueAsInt(la->items[i], context), ValueAsInt(lb->items[i], context)));
      }
      return out;
    }
    if (a_is_list) {
      ListPtr la = ValueAsListPtr(a, context);
      int scalar = ValueAsInt(b, context);
      out->items.reserve(la->items.size());
      for (const Value& v : la->items) {
        out->items.push_back(apply_op(ValueAsInt(v, context), scalar));
      }
      return out;
    }
    ListPtr lb = ValueAsListPtr(b, context);
    int scalar = ValueAsInt(a, context);
    out->items.reserve(lb->items.size());
    for (const Value& v : lb->items) {
      out->items.push_back(apply_op(scalar, ValueAsInt(v, context)));
    }
    return out;
  }

  static int TorchSigmoidPpm(int x) {
    const double xf = static_cast<double>(x) / 1000.0;
    const double s = 1.0 / (1.0 + std::exp(-xf));
    return static_cast<int>(std::round(s * 1000000.0));
  }

  static int TorchTanhPpm(int x) {
    const double xf = static_cast<double>(x) / 1000.0;
    const double t = std::tanh(xf);
    return static_cast<int>(std::round(t * 1000000.0));
  }

  static void ExpectArgc(const std::string& name, int argc, int expected) {
    if (argc != expected) {
      throw std::runtime_error(name + " expects " + std::to_string(expected) +
                               " args, got " + std::to_string(argc));
    }
  }

  std::vector<Value> stack_;
  std::unordered_map<std::string, Value> vars_;
  GraphicsState gfx_;
  Gx3dState gx3d_{gfx_};
  NetState net_;
  std::filesystem::path module_base_;
  std::uint32_t random_seed_ = 1337U;
  std::mt19937 rng_{random_seed_};
  std::uint32_t noise_seed_ = 12345U;
  std::uint32_t torch_seed_ = 4242U;
  std::mt19937 torch_rng_{torch_seed_};
};

std::string ReadFile(const std::filesystem::path& file) {
  std::ifstream stream(file, std::ios::binary);
  if (!stream) {
    throw std::runtime_error("Failed to open source file: " + file.string());
  }
  std::ostringstream buffer;
  buffer << stream.rdbuf();
  return buffer.str();
}

std::string EscapeBytecodeField(const std::string& value) {
  std::string out;
  out.reserve(value.size());
  for (char ch : value) {
    if (ch == '\\') {
      out += "\\\\";
    } else if (ch == '|') {
      out += "\\|";
    } else if (ch == '\n') {
      out += "\\n";
    } else {
      out.push_back(ch);
    }
  }
  return out;
}

std::string StripCarriageReturn(std::string value) {
  if (!value.empty() && value.back() == '\r') {
    value.pop_back();
  }
  return value;
}

std::vector<std::string> SplitEscapedFields(const std::string& line) {
  std::vector<std::string> fields;
  std::string current;
  bool escaping = false;
  for (char ch : line) {
    if (escaping) {
      if (ch == 'n') {
        current.push_back('\n');
      } else if (ch == '|' || ch == '\\') {
        current.push_back(ch);
      } else {
        current.push_back(ch);
      }
      escaping = false;
      continue;
    }
    if (ch == '\\') {
      escaping = true;
      continue;
    }
    if (ch == '|') {
      fields.push_back(current);
      current.clear();
      continue;
    }
    current.push_back(ch);
  }
  if (escaping) {
    throw std::runtime_error("Invalid escaped field in bytecode");
  }
  fields.push_back(current);
  return fields;
}

std::string SerializeBytecode(const std::vector<Instruction>& code) {
  std::ostringstream stream;
  stream << "PYPPBC1\n";
  for (const Instruction& ins : code) {
    stream << ins.op;
    for (const std::string& arg : ins.args) {
      stream << "|" << EscapeBytecodeField(arg);
    }
    stream << "\n";
  }
  return stream.str();
}

void WriteBytecode(const std::filesystem::path& out_file,
                   const std::vector<Instruction>& code) {
  if (out_file.has_parent_path()) {
    std::filesystem::create_directories(out_file.parent_path());
  }
  std::ofstream stream(out_file, std::ios::binary);
  if (!stream) {
    throw std::runtime_error("Failed to open output file: " + out_file.string());
  }
  stream << SerializeBytecode(code);
}

std::vector<Instruction> ReadBytecodeStream(std::istream& stream) {
  std::string line;
  if (!std::getline(stream, line)) {
    throw std::runtime_error("Empty bytecode stream");
  }
  line = StripCarriageReturn(std::move(line));
  if (line != "PYPPBC1") {
    throw std::runtime_error("Unsupported bytecode format header: " + line);
  }

  std::vector<Instruction> code;
  while (std::getline(stream, line)) {
    line = StripCarriageReturn(std::move(line));
    if (line.empty()) {
      continue;
    }
    std::vector<std::string> fields = SplitEscapedFields(line);
    if (fields.empty() || fields[0].empty()) {
      throw std::runtime_error("Invalid bytecode instruction line");
    }
    Instruction ins;
    ins.op = fields[0];
    for (std::size_t i = 1; i < fields.size(); ++i) {
      ins.args.push_back(fields[i]);
    }
    code.push_back(std::move(ins));
  }

  return code;
}

std::vector<Instruction> ReadBytecode(const std::filesystem::path& in_file) {
  std::ifstream stream(in_file, std::ios::binary);
  if (!stream) {
    throw std::runtime_error("Failed to open bytecode file: " + in_file.string());
  }
  return ReadBytecodeStream(stream);
}

std::optional<std::vector<Instruction>> ReadEmbeddedBytecode(
    const std::filesystem::path& exe_file) {
  const std::string marker = "PYPP_EMBED_BC1";
  std::ifstream stream(exe_file, std::ios::binary);
  if (!stream) {
    return std::nullopt;
  }
  std::vector<char> data((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
  if (data.size() < marker.size() + sizeof(std::uint64_t)) {
    return std::nullopt;
  }

  auto it = std::find_end(data.begin(), data.end(), marker.begin(), marker.end());
  if (it == data.end()) {
    return std::nullopt;
  }
  std::size_t marker_pos = static_cast<std::size_t>(std::distance(data.begin(), it));
  std::size_t size_pos = marker_pos + marker.size();
  if (size_pos + sizeof(std::uint64_t) > data.size()) {
    return std::nullopt;
  }
  std::uint64_t payload_size = 0;
  for (std::size_t i = 0; i < sizeof(std::uint64_t); ++i) {
    payload_size |=
        static_cast<std::uint64_t>(static_cast<unsigned char>(data[size_pos + i])) <<
        (8 * i);
  }
  std::size_t payload_pos = size_pos + sizeof(std::uint64_t);
  if (payload_pos + static_cast<std::size_t>(payload_size) > data.size()) {
    return std::nullopt;
  }

  std::string payload(data.data() + payload_pos,
                      data.data() + payload_pos + static_cast<std::size_t>(payload_size));
  std::istringstream payload_stream(payload);
  return ReadBytecodeStream(payload_stream);
}

void WriteStandaloneExe(const std::filesystem::path& self_exe,
                        const std::filesystem::path& out_exe,
                        const std::vector<Instruction>& code) {
  if (out_exe.has_parent_path()) {
    std::filesystem::create_directories(out_exe.parent_path());
  }
  std::filesystem::copy_file(self_exe, out_exe,
                             std::filesystem::copy_options::overwrite_existing);
  std::string payload = SerializeBytecode(code);
  const std::string marker = "PYPP_EMBED_BC1";

  std::ofstream out(out_exe, std::ios::binary | std::ios::app);
  if (!out) {
    throw std::runtime_error("Failed to write output exe: " + out_exe.string());
  }
  out.write(marker.data(), static_cast<std::streamsize>(marker.size()));
  std::uint64_t size = static_cast<std::uint64_t>(payload.size());
  for (std::size_t i = 0; i < sizeof(std::uint64_t); ++i) {
    char byte = static_cast<char>((size >> (8 * i)) & 0xFF);
    out.write(&byte, 1);
  }
  out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
}

#ifdef _WIN32
std::string EscapeSingleQuotedPowerShell(const std::string& input) {
  std::string out;
  out.reserve(input.size());
  for (char ch : input) {
    if (ch == '\'') {
      out += "''";
    } else {
      out.push_back(ch);
    }
  }
  return out;
}

void InstallPathForCurrentUser(const std::filesystem::path& dir) {
  std::filesystem::path abs = std::filesystem::absolute(dir);
  std::string dir_text = abs.string();
  std::string esc = EscapeSingleQuotedPowerShell(dir_text);
  std::string script =
      "$d='" + esc +
      "';"
      "$p=[Environment]::GetEnvironmentVariable('Path','User');"
      "if([string]::IsNullOrEmpty($p)){$parts=@()}else{$parts=$p -split ';' | "
      "Where-Object {$_ -ne ''}};"
      "if($parts -contains $d){Write-Output ('Path already contains: ' + $d);exit 0};"
      "$new=if($parts.Count -eq 0){$d}else{($parts + $d) -join ';'};"
      "[Environment]::SetEnvironmentVariable('Path',$new,'User');"
      "Write-Output ('Added to User PATH: ' + $d);"
      "Write-Output 'Open a new terminal to use `pypp` globally.';";

  std::string command =
      "powershell -NoProfile -ExecutionPolicy Bypass -Command \"" + script + "\"";
  int rc = std::system(command.c_str());
  if (rc != 0) {
    throw std::runtime_error(
        "Failed to update PATH automatically. Please add this directory manually: " +
        dir_text);
  }
}
#else
void InstallPathForCurrentUser(const std::filesystem::path&) {
  throw std::runtime_error(
      "install-path is currently only supported on Windows in this project.");
}
#endif

void PrintUsage() {
  std::cout << "pypp (C++ edition)\n";
  std::cout << "Usage:\n";
  std::cout << "  pypp build|compile <file.pypp> [--out <dir>]\n";
  std::cout << "  pypp compile-exe <file.pypp> [--out <file.exe>]\n";
  std::cout << "  pypp run <file.pypp>\n";
  std::cout << "  pypp run-bytecode <file.ppbc>\n";
  std::cout << "  pypp install-path [--dir <folder>]\n";
  std::cout << "  pypp version\n";
}

std::vector<Instruction> CompileSource(const std::filesystem::path& source_file) {
  std::string source = ReadFile(source_file);
  Lexer lexer(source);
  std::vector<Token> tokens = lexer.Tokenize();
  Parser parser(tokens);
  return parser.ParseProgram();
}

}  // namespace pypp

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      std::optional<std::vector<pypp::Instruction>> embedded =
          pypp::ReadEmbeddedBytecode(argv[0]);
      if (embedded.has_value()) {
        pypp::VM vm;
        vm.Execute(*embedded);
        return 0;
      }
      pypp::PrintUsage();
      return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "version") {
      std::cout << "pypp 0.4.0-cpp\n";
      return 0;
    }

    if (cmd == "build" || cmd == "compile") {
      if (argc < 3) {
        pypp::PrintUsage();
        return 1;
      }
      std::filesystem::path source = argv[2];
      std::filesystem::path out_dir = "build";
      for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--out" && i + 1 < argc) {
          out_dir = argv[++i];
        } else {
          throw std::runtime_error("Unknown build argument: " + arg);
        }
      }
      std::vector<pypp::Instruction> code = pypp::CompileSource(source);
      std::filesystem::path out_file = out_dir / (source.stem().string() + ".ppbc");
      pypp::WriteBytecode(out_file, code);
      std::cout << "Wrote " << out_file.string() << "\n";
      return 0;
    }

    if (cmd == "run") {
      if (argc < 3) {
        pypp::PrintUsage();
        return 1;
      }
      std::filesystem::path source = argv[2];
      std::vector<pypp::Instruction> code = pypp::CompileSource(source);
      pypp::VM vm(source.parent_path());
      vm.Execute(code);
      return 0;
    }

    if (cmd == "run-bytecode") {
      if (argc < 3) {
        pypp::PrintUsage();
        return 1;
      }
      std::filesystem::path bytecode_file = argv[2];
      std::vector<pypp::Instruction> code = pypp::ReadBytecode(bytecode_file);
      pypp::VM vm(std::filesystem::current_path());
      vm.Execute(code);
      return 0;
    }

    if (cmd == "compile-exe") {
      if (argc < 3) {
        pypp::PrintUsage();
        return 1;
      }
      std::filesystem::path source = argv[2];
      std::filesystem::path out_exe = source.stem().string() + ".exe";
      for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--out" && i + 1 < argc) {
          out_exe = argv[++i];
        } else {
          throw std::runtime_error("Unknown compile-exe argument: " + arg);
        }
      }
      std::vector<pypp::Instruction> code = pypp::CompileSource(source);
      pypp::WriteStandaloneExe(argv[0], out_exe, code);
      std::cout << "Wrote standalone executable " << out_exe.string() << "\n";
      return 0;
    }

    if (cmd == "install-path") {
      std::filesystem::path target_dir = std::filesystem::absolute(argv[0]).parent_path();
      for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dir" && i + 1 < argc) {
          target_dir = argv[++i];
        } else {
          throw std::runtime_error("Unknown install-path argument: " + arg);
        }
      }
      pypp::InstallPathForCurrentUser(target_dir);
      return 0;
    }

    pypp::PrintUsage();
    return 1;
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }
}
