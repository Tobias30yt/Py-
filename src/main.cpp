#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <cstdlib>

namespace pypp {

enum class TokenKind {
  Eof,
  Newline,
  Identifier,
  Number,
  String,
  Let,
  If,
  While,
  End,
  LParen,
  RParen,
  Comma,
  Dot,
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
      std::string path = Previous().lexeme;
      while (Match(TokenKind::Dot)) {
        Token part = Consume(TokenKind::Identifier, "Expected identifier after '.'");
        path += "." + part.lexeme;
      }

      if (Match(TokenKind::LParen)) {
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
        if (path.find('.') != std::string::npos) {
          throw std::runtime_error("Dotted name is only valid as function call at " +
                                   CurrentPos());
        }
        out.push_back(Instruction{"LOAD", {path}});
      }
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

using Value = std::variant<int, std::string>;

std::string ValueToString(const Value& value) {
  if (std::holds_alternative<int>(value)) {
    return std::to_string(std::get<int>(value));
  }
  return std::get<std::string>(value);
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
  return !std::get<std::string>(value).empty();
}

struct Pixel {
  int r = 0;
  int g = 0;
  int b = 0;
};

struct GraphicsState {
  int width = 0;
  int height = 0;
  std::vector<Pixel> pixels;

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

 private:
  void EnsureOpen(const std::string& fn) const {
    if (!IsOpen()) {
      throw std::runtime_error(fn + " called before gfx.open");
    }
  }

  static int ClampColor(int v) { return std::max(0, std::min(255, v)); }
};

class VM {
 public:
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
      } else {
        throw std::runtime_error("Unknown opcode: " + ins.op);
      }
      ip += 1;
    }
  }

 private:
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

    throw std::runtime_error("Unknown function: " + name);
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

void WriteBytecode(const std::filesystem::path& out_file,
                   const std::vector<Instruction>& code) {
  if (out_file.has_parent_path()) {
    std::filesystem::create_directories(out_file.parent_path());
  }
  std::ofstream stream(out_file, std::ios::binary);
  if (!stream) {
    throw std::runtime_error("Failed to open output file: " + out_file.string());
  }
  stream << "PYPPBC1\n";
  for (const Instruction& ins : code) {
    stream << ins.op;
    for (const std::string& arg : ins.args) {
      stream << "|" << EscapeBytecodeField(arg);
    }
    stream << "\n";
  }
}

std::vector<Instruction> ReadBytecode(const std::filesystem::path& in_file) {
  std::ifstream stream(in_file, std::ios::binary);
  if (!stream) {
    throw std::runtime_error("Failed to open bytecode file: " + in_file.string());
  }

  std::string line;
  if (!std::getline(stream, line)) {
    throw std::runtime_error("Empty bytecode file: " + in_file.string());
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
      pypp::PrintUsage();
      return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "version") {
      std::cout << "pypp 0.3.0-cpp\n";
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
      pypp::VM vm;
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
      pypp::VM vm;
      vm.Execute(code);
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
