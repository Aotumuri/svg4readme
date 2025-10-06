<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/waves.svg" />

# svg4readme

## 概要
`svg4readme` は、夜空・月・波・ブランド名などを合成した SVG バナーを生成するプログラムです。
README のヘッダー画像やプロフィール装飾に利用できます。

## 特徴
Waves
- 星の瞬きと流れ星アニメーション
- グラデーションの空と発光する月
- 複数レイヤーの波のゆらぎ
- 名前や使用言語などを動的に出力

## ビルド方法
config.cを編集したら以下のコマンドを実行する必要があります。
```bash
gcc -O2 waves_svg.c config.c -lm -o waves_svg
```

## 実行方法
読み込み：
```bash
./waves_svg > banner.svg
```

## 出力例
生成例：
<img src="https://raw.githubusercontent.com/Aotumuri/svg4readme/refs/heads/main/sample/waves.svg" />