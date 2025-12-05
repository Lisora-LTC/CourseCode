# 自动为 Client.h 中的所有函数定义添加 inline 关键字
param(
    [string]$FilePath = "Client.h"
)

# 读取文件内容
$content = Get-Content $FilePath -Raw -Encoding UTF8

# 替换所有类成员函数定义,添加 inline 关键字
# 匹配模式: bool ClientHelper::function_name 等
$patterns = @(
    'bool ClientHelper::',
    'void ClientHelper::',
    'unsigned int ClientHelper::',
    'DWORD __stdcall ClientHelper::'
)

foreach ($pattern in $patterns) {
    # 检查是否已经有 inline
    $content = $content -replace "(?<!inline )($pattern)", 'inline $1'
}

# 保存文件
[System.IO.File]::WriteAllText($FilePath, $content, [System.Text.UTF8Encoding]::new($false))

Write-Host "Done adding inline keywords to $FilePath" -ForegroundColor Green
