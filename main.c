import yfinance as yf
import matplotlib.pyplot as plt
import pandas as pd
import os

def run_final_finance_system(stock_code, start_date, end_date, initial_investment=100000):
    """
    個人化投資組合自動化分析與報表系統
    學號：B1445019
    """
    print("=" * 50)
    print(f"系統啟動：開始分析股票 [{stock_code}]")
    print(f"分析區間：{start_date} 至 {end_date}")
    print(f"初始投入本金：${initial_investment:,.0f} 元")
    print("=" * 50)
    
    # 1. 數據獲取 (抓取標準 OHLCV 數據集)
    try:
        df = yf.download(stock_code, start=start_date, end=end_date)
    except Exception as e:
        print(f"連線失敗或代碼錯誤: {e}")
        return
        
    if df.empty:
        print("錯誤：未抓取到有效數據，請檢查股票代碼或日期。")
        return

    # 解決 yfinance 新版本帶來的 MultiIndex 欄位問題
    if isinstance(df.columns, pd.MultiIndex):
        # 如果是多重索引，只保留最底層的欄位名稱 (例如：'Close', 'Open')
        df.columns = df.columns.get_level_values(0)
    else:
        # 如果是普通欄位但有名稱多餘的狀況，將其扁平化
        df.columns = [col[0] if isinstance(col, tuple) else col for col in df.columns]

    # 2. 數據清洗與財務運算
    # 優先使用調整後收盤價 (Adj Close)，若無則用收盤價 (Close)
    if 'Adj Close' in df.columns:
        price_col = 'Adj Close'
    elif 'Close' in df.columns:
        price_col = 'Close'
    else:
        # 萬一 yfinance 回傳的欄位是小寫，做個保險
        df.columns = [c.lower() for c in df.columns]
        price_col = 'adj close' if 'adj close' in df.columns else 'close'
        
    # 清洗缺失值 (Data Cleaning)
    df = df.dropna(subset=[price_col])
    
    # 計算累積報酬率 (Cumulative Return)
    initial_price = float(df[price_col].iloc[0])
    df['Cumulative_Return_Pct'] = (df[price_col] / initial_price - 1) * 100
    
    # 計算資產現值 (Current Asset Value)
    df['Asset_Value'] = initial_investment * (df[price_col] / initial_price)

    # 3. 系統輸出成果一：自動化財務報表匯出 (Excel)
    excel_filename = f"{stock_code}_財務分析報告.xlsx"
    df.to_excel(excel_filename)
    print(f"\n[成功] 財務數據已自動整合，並匯出至 Excel 檔案: '{excel_filename}'")

    # 4. 系統輸出成果二：數據視覺化 (雙層對照圖)
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(11, 7), sharex=True)

    # 上圖：歷史股價走勢
    ax1.plot(df.index, df[price_col], color='#1f77b4', linewidth=2, label='Close Price')
    ax1.set_title(f"Financial Portfolio Performance Analysis - {stock_code}", fontsize=14, fontweight='bold')
    ax1.set_ylabel("Stock Price ($)", fontsize=11)
    ax1.legend(loc='upper left')

    # 下圖：累積報酬率 (%)
    ax2.plot(df.index, df['Cumulative_Return_Pct'], color='#2ca02c', linewidth=2, label='Cumulative Return (%)')
    ax2.axhline(y=0, color='red', linestyle='--', linewidth=1.2, label='Break-even Line (0%)') # 盈虧平衡線
    ax2.set_ylabel("Cumulative Return (%)", fontsize=11)
    ax2.set_xlabel("Date", fontsize=11)
    ax2.legend(loc='upper left')

    # 終端機控制台績效總結摘要
    final_return = float(df['Cumulative_Return_Pct'].iloc[-1])
    final_value = float(df['Asset_Value'].iloc[-1])
    net_profit = final_value - initial_investment
    
    print("-" * 50)
    print(f"【分析績效摘要】")
    print(f"▶ 期初資產價值: ${initial_investment:,.0f}")
    print(f"▶ 期末資產現值: ${final_value:,.0f}")
    print(f"▶ 淨損益 (Net Profit): ${net_profit:,.0f}")
    print(f"▶ 最終累積報酬率: {final_return:.2f}%")
    print("-" * 50)

    plt.tight_layout()
    
    # 自動儲存圖表為 PNG 圖片
    image_filename = f"{stock_code}_趨勢圖.png"
    plt.savefig(image_filename, dpi=300)
    print(f"[成功] 視覺化趨勢圖已儲存: '{image_filename}'\n")
    
    plt.show()

# === 展示最終成果運作 ===
run_final_finance_system(stock_code="0050.TW", start_date="2025-01-01", end_date="2026-05-01", initial_investment=100000)
