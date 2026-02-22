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
#include <thread>
#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <cstdlib>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
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
using ObjectPtr = std::shared_ptr<Object>;
using Value = std::variant<int, std::string, ObjectPtr>;

struct Object {
  std::unordered_map<std::string, Value> fields;
};

std::string ValueToString(const Value& value) {
  if (std::holds_alternative<int>(value)) {
    return std::to_string(std::get<int>(value));
  }
  if (std::holds_alternative<std::string>(value)) {
    return std::get<std::string>(value);
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
#ifdef _WIN32
  HWND hwnd = nullptr;
  bool window_open = false;
  std::array<bool, 256> key_state{};
  std::vector<std::uint32_t> rgba_buffer;
#endif

  bool IsOpen() const { return width > 0 && height > 0; }

  void Open(int w, int h) {
    if (w <= 0 || h <= 0) {
      throw std::runtime_error("gfx.open expects positive width/height");
    }
    width = w;
    height = h;
    pixels.assign(static_cast<std::size_t>(w * h), Pixel{0, 0, 0});
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
    rgba_buffer.assign(static_cast<std::size_t>(width * height), 0);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
#else
    (void)title;
    throw std::runtime_error("Live windowing currently supported on Windows only");
#endif
  }

  int PollEvents() {
#ifdef _WIN32
    if (!window_open) {
      return 0;
    }
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
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
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        const Pixel& p = pixels[static_cast<std::size_t>(y * width + x)];
        std::uint32_t value = (static_cast<std::uint32_t>(p.b) << 16) |
                              (static_cast<std::uint32_t>(p.g) << 8) |
                              static_cast<std::uint32_t>(p.r);
        rgba_buffer[static_cast<std::size_t>(y * width + x)] = value;
      }
    }

    HDC hdc = GetDC(hwnd);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height,
                  rgba_buffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd, hdc);
    return 1;
#else
    return 0;
#endif
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
    if (hwnd) {
      DestroyWindow(hwnd);
      hwnd = nullptr;
    }
    window_open = false;
#endif
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

#ifdef _WIN32
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
        DestroyWindow(hwnd_handle);
        return 0;
      case WM_DESTROY:
        window_open = false;
        return 0;
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
    explicit Gx3dState(GraphicsState& gfx) : gfx_(gfx) {}

    void Reset() {
      cam_ = Vec3{0.0, 0.0, -220.0};
      rot_deg_ = Vec3{0.0, 0.0, 0.0};
      fov_ = 300.0;
    }

    void Camera(int x, int y, int z) { cam_ = Vec3{static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)}; }

    void Rotate(int x_deg, int y_deg, int z_deg) {
      rot_deg_ = Vec3{static_cast<double>(x_deg), static_cast<double>(y_deg), static_cast<double>(z_deg)};
    }

    void Fov(int fov) {
      if (fov <= 10) {
        throw std::runtime_error("gx3d.fov expects value > 10");
      }
      fov_ = static_cast<double>(fov);
    }

    void Cube(int cx, int cy, int cz, int size, int r, int g, int b) {
      if (!gfx_.IsOpen()) {
        throw std::runtime_error("gx3d.cube requires gfx.open(...) or gfx.window(...) first");
      }
      if (size <= 0) {
        return;
      }

      const double h = static_cast<double>(size) / 2.0;
      std::array<Vec3, 8> verts = {
          Vec3{-h, -h, -h}, Vec3{h, -h, -h},  Vec3{h, h, -h},  Vec3{-h, h, -h},
          Vec3{-h, -h, h},  Vec3{h, -h, h},   Vec3{h, h, h},   Vec3{-h, h, h},
      };

      for (Vec3& v : verts) {
        v = RotateVec(v, rot_deg_);
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
          gfx_.Line(p1->first, p1->second, p2->first, p2->second, r, g, b);
        }
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

    std::optional<std::pair<int, int>> Project(const Vec3& world) const {
      const double x = world.x - cam_.x;
      const double y = world.y - cam_.y;
      const double z = world.z - cam_.z;
      if (z <= 1.0) {
        return std::nullopt;
      }

      const double sx = (x / z) * fov_ + static_cast<double>(gfx_.Width()) / 2.0;
      const double sy = (-y / z) * fov_ + static_cast<double>(gfx_.Height()) / 2.0;
      return std::pair<int, int>{static_cast<int>(std::round(sx)),
                                 static_cast<int>(std::round(sy))};
    }

    GraphicsState& gfx_;
    Vec3 cam_{0.0, 0.0, -220.0};
    Vec3 rot_deg_{0.0, 0.0, 0.0};
    double fov_ = 300.0;
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

    if (name == "gfx.open") {
      ExpectArgc(name, argc, 2);
      gfx_.Open(ValueAsInt(args[0], name), ValueAsInt(args[1], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gfx.clear") {
      ExpectArgc(name, argc, 3);
      gfx_.Clear(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                 ValueAsInt(args[2], name));
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
      return;
    }
    if (name == "gfx.key_down") {
      ExpectArgc(name, argc, 1);
      stack_.push_back(gfx_.KeyDown(ValueAsInt(args[0], name)));
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
    if (name == "gx3d.rotate") {
      ExpectArgc(name, argc, 3);
      gx3d_.Rotate(ValueAsInt(args[0], name), ValueAsInt(args[1], name),
                   ValueAsInt(args[2], name));
      stack_.push_back(0);
      return;
    }
    if (name == "gx3d.fov") {
      ExpectArgc(name, argc, 1);
      gx3d_.Fov(ValueAsInt(args[0], name));
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
  std::filesystem::path module_base_;
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
