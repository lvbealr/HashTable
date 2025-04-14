import re

with open("lesMiserables.txt", "r", encoding="utf-8") as f:
    text = f.read()

words = re.findall(r"\b[a-zA-Z']+\b", text)
words = [word.lower() for word in words if 5 <= len(word) <= 32]
words = [word for word in words if word.strip()]

with open("text.txt", "w", encoding="utf-8") as f:
    f.write("\n".join(words))