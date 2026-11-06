#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>
#include <utility>
#include <map>
#include <sstream>
#include <fstream>


// ============================== 1. ENUMS ====================================

enum class Category { Men, Women, Other };

std::string categoryToString(Category c) {
    switch (c) {
        case Category::Men:   return "Men";
        case Category::Women: return "Women";
        case Category::Other: return "Other";
    }
    return "Unknown";
}

// Parses the persisted string form of a Category back into the enum.
// Falls back to Other for anything unrecognized (defensive against a
// hand-edited or corrupted inventory.txt).
Category categoryFromString(const std::string& s) {
    if (s == "Men")   return Category::Men;
    if (s == "Women") return Category::Women;
    return Category::Other;
}

// The 4 Myanmar mobile-banking payment methods and internitional payment methods required by the project.
// (Replaces the earlier PayPal/Payoneer/GPay/ApplePay set.)
enum class PaymentMethod { KPay, AyaPay, CBPay, WavePay, PayPal, Payoneer, GPay, ApplePay };

// The 2 account roles (project requirement: Role-Based Access Control).
// Admin  -> full access: every customer feature PLUS the Admin Panel.
// Customer -> shopping features only; the Admin Panel is never shown/reachable.
enum class Role { Admin, Customer };

std::string roleToString(Role r) {
    switch (r) {
        case Role::Admin:    return "Admin";
        case Role::Customer: return "Customer";
    }
    return "Customer";
}

Role roleFromString(const std::string& s) {
    return (s == "Admin") ? Role::Admin : Role::Customer;
}

// ========================== 1.5 LANGUAGE / CURRENCY =========================

// The 5 supported UI languages.
enum class Language { English, Chinese, Japanese, Italian, French };

// The 5 supported display currencies. Catalog prices are stored in USD;
// everything else is converted for display only. MMK (Myanmar Kyat) was
// added to support the Myanmar mobile-banking checkout flow.
enum class Currency { USD, CNY, JPY, EUR, MMK };

// Two-letter code used both for menus and as the Localization table key prefix.
std::string languageCode(Language lang) {
    switch (lang) {
        case Language::English:  return "en";
        case Language::Chinese:  return "ch";
        case Language::Japanese: return "jp";
        case Language::Italian:  return "it";
        case Language::French:   return "fr";
    }
    return "en";
}

// "English (en)" style label shown in the Settings sub-menu.
std::string languageDisplayName(Language lang) {
    switch (lang) {
        case Language::English:  return "English (en)";
        case Language::Chinese:  return "Chinese (ch)";
        case Language::Japanese: return "Japanese (jp)";
        case Language::Italian:  return "Italian (it)";
        case Language::French:   return "French (fr)";
    }
    return "English (en)";
}

// Regional mapping: selecting a language auto-selects its currency.
// it/fr both map to EUR since they're both Eurozone locales. There is
// intentionally no language mapped to MMK -- Myanmar Kyat is offered as
// an explicit, manually-selected currency instead (see handleCurrencyMenu).
Currency currencyForLanguage(Language lang) {
    switch (lang) {
        case Language::English:  return Currency::USD;
        case Language::Chinese:  return Currency::CNY;
        case Language::Japanese: return Currency::JPY;
        case Language::Italian:  return Currency::EUR;
        case Language::French:   return Currency::EUR;
    }
    return Currency::USD;
}

std::string currencyCode(Currency c) {
    switch (c) {
        case Currency::USD: return "USD";
        case Currency::CNY: return "CNY";
        case Currency::JPY: return "JPY";
        case Currency::EUR: return "EUR";
        case Currency::MMK: return "MMK";
    }
    return "USD";
}

std::string currencySymbol(Currency c) {
    switch (c) {
        case Currency::USD: return "$";
        case Currency::CNY: return "\xC2\xA5"; // ¥
        case Currency::JPY: return "\xC2\xA5"; // ¥
        case Currency::EUR: return "\xE2\x82\xAC"; // €
        case Currency::MMK: return "K"; // Conventional ASCII Kyat abbreviation (Ks./K)
    }
    return "$";
}

// "USD ($)" style label shown in the Settings sub-menu.
std::string currencyDisplayName(Currency c) {
    return currencyCode(c) + " (" + currencySymbol(c) + ")";
}

// Fixed illustrative conversion rates FROM USD (catalog prices are USD).
// A production build would refresh these from a live FX feed.
// 1 USD = 2100 MMK is used as the configurable Myanmar Kyat rate.
double exchangeRateFromUSD(Currency c) {
    switch (c) {
        case Currency::USD: return 1.0;
        case Currency::CNY: return 7.20;
        case Currency::JPY: return 157.0;
        case Currency::EUR: return 0.93;
        case Currency::MMK: return 2100.0;
    }
    return 1.0;
}

// Maps a Category to the same translation key used for the Men/Women/Other
// menu entries, so product listings and menus stay in sync automatically.
std::string categoryLocalizationKey(Category c) {
    switch (c) {
        case Category::Men:   return "menu_men";
        case Category::Women: return "menu_women";
        case Category::Other: return "menu_other";
    }
    return "menu_other";
}

// Small string table for every piece of UI text that changes with the
// selected language. Keyed as "<language-code>.<key>" so lookups are a
// single map access; falls back to English if a key is ever missing.
class Localization {
public:
    static std::string text(const std::string& key, Language lang) {
        const auto& table = table_();
        auto it = table.find(languageCode(lang) + "." + key);
        if (it != table.end()) {
            return it->second;
        }
        auto fallback = table.find("en." + key);
        return fallback != table.end() ? fallback->second : key;
    }

private:
    static const std::map<std::string, std::string>& table_() {
        static const std::map<std::string, std::string> table = {
            {"en.login_title", "===== Watch Store Login ====="},
            {"ch.login_title", "===== 手表商店登录 ====="},
            {"jp.login_title", "===== ウォッチストア ログイン ====="},
            {"it.login_title", "===== Accesso al Negozio di Orologi ====="},
            {"fr.login_title", "===== Connexion à la Boutique de Montres ====="},

            {"en.username_prompt", "Username: "},
            {"ch.username_prompt", "用户名: "},
            {"jp.username_prompt", "ユーザー名: "},
            {"it.username_prompt", "Nome utente: "},
            {"fr.username_prompt", "Nom d'utilisateur : "},

            {"en.password_prompt", "Password: "},
            {"ch.password_prompt", "密码: "},
            {"jp.password_prompt", "パスワード: "},
            {"it.password_prompt", "Password: "},
            {"fr.password_prompt", "Mot de passe : "},

            {"en.welcome", "Welcome, "},
            {"ch.welcome", "欢迎, "},
            {"jp.welcome", "ようこそ, "},
            {"it.welcome", "Benvenuto, "},
            {"fr.welcome", "Bienvenue, "},

            {"en.invalid_credentials", "Invalid credentials. Attempts left: "},
            {"ch.invalid_credentials", "凭证无效。剩余尝试次数: "},
            {"jp.invalid_credentials", "認証情報が無効です。残り試行回数: "},
            {"it.invalid_credentials", "Credenziali non valide. Tentativi rimasti: "},
            {"fr.invalid_credentials", "Identifiants invalides. Tentatives restantes : "},

            {"en.too_many_attempts", "Too many failed attempts. Exiting."},
            {"ch.too_many_attempts", "失败次数过多，正在退出。"},
            {"jp.too_many_attempts", "失敗回数が多すぎます。終了します。"},
            {"it.too_many_attempts", "Troppi tentativi falliti. Uscita in corso."},
            {"fr.too_many_attempts", "Trop de tentatives échouées. Fermeture."},

            {"en.main_menu_title", "===== Main Menu ====="},
            {"ch.main_menu_title", "===== 主菜单 ====="},
            {"jp.main_menu_title", "===== メインメニュー ====="},
            {"it.main_menu_title", "===== Menu Principale ====="},
            {"fr.main_menu_title", "===== Menu Principal ====="},

            {"en.menu_home", "Home"},
            {"ch.menu_home", "首页"},
            {"jp.menu_home", "ホーム"},
            {"it.menu_home", "Home"},
            {"fr.menu_home", "Accueil"},

            {"en.menu_brand", "Brand"},
            {"ch.menu_brand", "品牌"},
            {"jp.menu_brand", "ブランド"},
            {"it.menu_brand", "Marca"},
            {"fr.menu_brand", "Marque"},

            {"en.menu_men", "Men"},
            {"ch.menu_men", "男士"},
            {"jp.menu_men", "メンズ"},
            {"it.menu_men", "Uomo"},
            {"fr.menu_men", "Homme"},

            {"en.menu_women", "Women"},
            {"ch.menu_women", "女士"},
            {"jp.menu_women", "レディース"},
            {"it.menu_women", "Donna"},
            {"fr.menu_women", "Femme"},

            {"en.menu_other", "Other"},
            {"ch.menu_other", "其他"},
            {"jp.menu_other", "その他"},
            {"it.menu_other", "Altro"},
            {"fr.menu_other", "Autre"},

            {"en.menu_search", "Search by Brand"},
            {"ch.menu_search", "按品牌搜索"},
            {"jp.menu_search", "ブランドで検索"},
            {"it.menu_search", "Cerca per Marca"},
            {"fr.menu_search", "Rechercher par Marque"},

            {"en.menu_cart", "View Cart"},
            {"ch.menu_cart", "查看购物车"},
            {"jp.menu_cart", "カートを見る"},
            {"it.menu_cart", "Visualizza Carrello"},
            {"fr.menu_cart", "Voir le Panier"},

            {"en.menu_checkout", "Checkout"},
            {"ch.menu_checkout", "结账"},
            {"jp.menu_checkout", "チェックアウト"},
            {"it.menu_checkout", "Checkout"},
            {"fr.menu_checkout", "Paiement"},

            {"en.menu_settings", "Settings"},
            {"ch.menu_settings", "设置"},
            {"jp.menu_settings", "設定"},
            {"it.menu_settings", "Impostazioni"},
            {"fr.menu_settings", "Paramètres"},

            {"en.menu_exit", "Exit"},
            {"ch.menu_exit", "退出"},
            {"jp.menu_exit", "終了"},
            {"it.menu_exit", "Esci"},
            {"fr.menu_exit", "Quitter"},

            {"en.goodbye", "Thank you for visiting, "},
            {"ch.goodbye", "感谢您的光临, "},
            {"jp.goodbye", "ご来店ありがとうございました, "},
            {"it.goodbye", "Grazie per la visita, "},
            {"fr.goodbye", "Merci de votre visite, "},

            {"en.goodbye_suffix", ". Goodbye!"},
            {"ch.goodbye_suffix", "。再见！"},
            {"jp.goodbye_suffix", "。さようなら！"},
            {"it.goodbye_suffix", ". Arrivederci!"},
            {"fr.goodbye_suffix", ". Au revoir !"},

            {"en.settings_title", "--- Settings ---"},
            {"ch.settings_title", "--- 设置 ---"},
            {"jp.settings_title", "--- 設定 ---"},
            {"it.settings_title", "--- Impostazioni ---"},
            {"fr.settings_title", "--- Paramètres ---"},

            {"en.settings_current_lang", "Current language: "},
            {"ch.settings_current_lang", "当前语言: "},
            {"jp.settings_current_lang", "現在の言語: "},
            {"it.settings_current_lang", "Lingua attuale: "},
            {"fr.settings_current_lang", "Langue actuelle : "},

            {"en.settings_current_currency", "Current currency: "},
            {"ch.settings_current_currency", "当前货币: "},
            {"jp.settings_current_currency", "現在の通貨: "},
            {"it.settings_current_currency", "Valuta attuale: "},
            {"fr.settings_current_currency", "Devise actuelle : "},

            {"en.settings_choose", "Choose a language:"},
            {"ch.settings_choose", "选择语言："},
            {"jp.settings_choose", "言語を選択してください："},
            {"it.settings_choose", "Scegli una lingua:"},
            {"fr.settings_choose", "Choisissez une langue :"},

            {"en.settings_back", "Back"},
            {"ch.settings_back", "返回"},
            {"jp.settings_back", "戻る"},
            {"it.settings_back", "Indietro"},
            {"fr.settings_back", "Retour"},

            {"en.settings_updated", "Language set to "},
            {"ch.settings_updated", "语言已设置为 "},
            {"jp.settings_updated", "言語が次のように設定されました: "},
            {"it.settings_updated", "Lingua impostata su "},
            {"fr.settings_updated", "Langue définie sur "},

            {"en.settings_currency_auto", " -- currency automatically set to "},
            {"ch.settings_currency_auto", " -- 货币已自动设置为 "},
            {"jp.settings_currency_auto", " -- 通貨は自動的に次のように設定されました: "},
            {"it.settings_currency_auto", " -- valuta impostata automaticamente su "},
            {"fr.settings_currency_auto", " -- devise automatiquement définie sur "},

            {"en.no_watches_found", "No watches found."},
            {"ch.no_watches_found", "未找到手表。"},
            {"jp.no_watches_found", "時計が見つかりません。"},
            {"it.no_watches_found", "Nessun orologio trovato."},
            {"fr.no_watches_found", "Aucune montre trouvée."},

            {"en.add_to_cart_prompt", "Enter the ID of a watch to add to cart, or 0 to go back: "},
            {"ch.add_to_cart_prompt", "输入要加入购物车的手表ID，或输入0返回："},
            {"jp.add_to_cart_prompt", "カートに追加する時計のIDを入力するか、0で戻る："},
            {"it.add_to_cart_prompt", "Inserisci l'ID di un orologio da aggiungere al carrello, o 0 per tornare indietro: "},
            {"fr.add_to_cart_prompt", "Entrez l'ID d'une montre à ajouter au panier, ou 0 pour revenir : "},

            {"en.cart_title", "--- Your Cart ---"},
            {"ch.cart_title", "--- 您的购物车 ---"},
            {"jp.cart_title", "--- あなたのカート ---"},
            {"it.cart_title", "--- Il Tuo Carrello ---"},
            {"fr.cart_title", "--- Votre Panier ---"},

            {"en.cart_empty", "Your cart is empty."},
            {"ch.cart_empty", "您的购物车是空的。"},
            {"jp.cart_empty", "カートは空です。"},
            {"it.cart_empty", "Il tuo carrello è vuoto."},
            {"fr.cart_empty", "Votre panier est vide."},

            {"en.cart_total", "Total: "},
            {"ch.cart_total", "总计: "},
            {"jp.cart_total", "合計: "},
            {"it.cart_total", "Totale: "},
            {"fr.cart_total", "Total : "},

            // ---- Admin Panel + quantity flow ----
            {"en.menu_admin", "Admin Panel"},
            {"ch.menu_admin", "管理面板"},
            {"jp.menu_admin", "管理パネル"},
            {"it.menu_admin", "Pannello Amministratore"},
            {"fr.menu_admin", "Panneau d'Administration"},

            {"en.quantity_prompt", "Enter quantity (default 1): "},
            {"ch.quantity_prompt", "输入数量（默认1）："},
            {"jp.quantity_prompt", "数量を入力してください（デフォルト1）："},
            {"it.quantity_prompt", "Inserisci la quantità (predefinito 1): "},
            {"fr.quantity_prompt", "Entrez la quantité (par défaut 1) : "},

            {"en.admin_panel_title", "===== Admin Panel ====="},
            {"ch.admin_panel_title", "===== 管理面板 ====="},
            {"jp.admin_panel_title", "===== 管理パネル ====="},
            {"it.admin_panel_title", "===== Pannello Amministratore ====="},
            {"fr.admin_panel_title", "===== Panneau d'Administration ====="},

            {"en.admin_option_users", "Manage User Accounts"},
            {"ch.admin_option_users", "管理用户账户"},
            {"jp.admin_option_users", "ユーザーアカウント管理"},
            {"it.admin_option_users", "Gestisci Account Utente"},
            {"fr.admin_option_users", "Gérer les Comptes Utilisateurs"},

            {"en.admin_option_inventory", "Manage Product Inventory"},
            {"ch.admin_option_inventory", "管理产品库存"},
            {"jp.admin_option_inventory", "商品在庫管理"},
            {"it.admin_option_inventory", "Gestisci Inventario Prodotti"},
            {"fr.admin_option_inventory", "Gérer l'Inventaire des Produits"},

            {"en.admin_back_to_main", "Back to Main Menu"},
            {"ch.admin_back_to_main", "返回主菜单"},
            {"jp.admin_back_to_main", "メインメニューに戻る"},
            {"it.admin_back_to_main", "Torna al Menu Principale"},
            {"fr.admin_back_to_main", "Retour au Menu Principal"},

            // ---- Currency menu + shipping address flow ----
            {"en.menu_currency", "Currency"},
            {"ch.menu_currency", "货币"},
            {"jp.menu_currency", "通貨"},
            {"it.menu_currency", "Valuta"},
            {"fr.menu_currency", "Devise"},

            {"en.currency_title", "--- Currency ---"},
            {"ch.currency_title", "--- 货币 ---"},
            {"jp.currency_title", "--- 通貨 ---"},
            {"it.currency_title", "--- Valuta ---"},
            {"fr.currency_title", "--- Devise ---"},

            {"en.currency_choose", "Choose a display currency:"},
            {"ch.currency_choose", "选择显示货币："},
            {"jp.currency_choose", "表示通貨を選択してください："},
            {"it.currency_choose", "Scegli una valuta di visualizzazione:"},
            {"fr.currency_choose", "Choisissez une devise d'affichage :"},

            {"en.shipping_title", "--- Shipping Address ---"},
            {"ch.shipping_title", "--- 收货地址 ---"},
            {"jp.shipping_title", "--- 配送先住所 ---"},
            {"it.shipping_title", "--- Indirizzo di Spedizione ---"},
            {"fr.shipping_title", "--- Adresse de Livraison ---"},

            {"en.shipping_recipient", "Recipient name: "},
            {"ch.shipping_recipient", "收件人姓名："},
            {"jp.shipping_recipient", "受取人名："},
            {"it.shipping_recipient", "Nome del destinatario: "},
            {"fr.shipping_recipient", "Nom du destinataire : "},

            {"en.shipping_phone", "Phone number: "},
            {"ch.shipping_phone", "电话号码："},
            {"jp.shipping_phone", "電話番号："},
            {"it.shipping_phone", "Numero di telefono: "},
            {"fr.shipping_phone", "Numéro de téléphone : "},

            {"en.shipping_address", "Shipping address / city: "},
            {"ch.shipping_address", "收货地址/城市："},
            {"jp.shipping_address", "配送先住所/市区町村："},
            {"it.shipping_address", "Indirizzo di spedizione / città: "},
            {"fr.shipping_address", "Adresse de livraison / ville : "},

            {"en.order_summary_title", "===== Order Summary ====="},
            {"ch.order_summary_title", "===== 订单摘要 ====="},
            {"jp.order_summary_title", "===== 注文概要 ====="},
            {"it.order_summary_title", "===== Riepilogo Ordine ====="},
            {"fr.order_summary_title", "===== Récapitulatif de la Commande ====="},

            {"en.currency_set_to", "Currency set to "},
            {"ch.currency_set_to", "货币已设置为 "},
            {"jp.currency_set_to", "通貨が次に設定されました： "},
            {"it.currency_set_to", "Valuta impostata su "},
            {"fr.currency_set_to", "Devise définie sur "},

            // ---- Checkout: proceed-to-payment flow ----
            {"en.checkout_cart_empty", "Your cart is empty. Add items before checking out."},
            {"ch.checkout_cart_empty", "您的购物车是空的。请先添加商品再结账。"},
            {"jp.checkout_cart_empty", "カートが空です。チェックアウトの前に商品を追加してください。"},
            {"it.checkout_cart_empty", "Il carrello è vuoto. Aggiungi articoli prima di procedere al checkout."},
            {"fr.checkout_cart_empty", "Votre panier est vide. Ajoutez des articles avant de passer à la caisse."},

            {"en.checkout_proceed_payment", "Proceed to payment? (1 = Yes, 2 = No)"},
            {"ch.checkout_proceed_payment", "是否继续付款？（1 = 是，2 = 否）"},
            {"jp.checkout_proceed_payment", "支払いに進みますか？（1 = はい、2 = いいえ）"},
            {"it.checkout_proceed_payment", "Procedere al pagamento? (1 = Sì, 2 = No)"},
            {"fr.checkout_proceed_payment", "Procéder au paiement ? (1 = Oui, 2 = Non)"},

            {"en.checkout_payment_method", "Payment method: "},
            {"ch.checkout_payment_method", "支付方式："},
            {"jp.checkout_payment_method", "支払い方法："},
            {"it.checkout_payment_method", "Metodo di pagamento: "},
            {"fr.checkout_payment_method", "Mode de paiement : "},

            {"en.checkout_amount_charged", "Amount charged: "},
            {"ch.checkout_amount_charged", "扣款金额："},
            {"jp.checkout_amount_charged", "請求金額："},
            {"it.checkout_amount_charged", "Importo addebitato: "},
            {"fr.checkout_amount_charged", "Montant débité : "},

            // ---- View Cart menu + update/remove flow ----
            {"en.cart_menu_update_qty", "Update quantity"},
            {"ch.cart_menu_update_qty", "更新数量"},
            {"jp.cart_menu_update_qty", "数量を更新"},
            {"it.cart_menu_update_qty", "Aggiorna quantità"},
            {"fr.cart_menu_update_qty", "Mettre à jour la quantité"},

            {"en.cart_menu_remove_item", "Remove an item"},
            {"ch.cart_menu_remove_item", "移除商品"},
            {"jp.cart_menu_remove_item", "商品を削除"},
            {"it.cart_menu_remove_item", "Rimuovi un articolo"},
            {"fr.cart_menu_remove_item", "Supprimer un article"},

            {"en.cart_update_id_prompt", "Enter watch ID to update: "},
            {"ch.cart_update_id_prompt", "输入要更新的手表ID："},
            {"jp.cart_update_id_prompt", "更新する時計のIDを入力してください："},
            {"it.cart_update_id_prompt", "Inserisci l'ID dell'orologio da aggiornare: "},
            {"fr.cart_update_id_prompt", "Entrez l'ID de la montre à mettre à jour : "},

            {"en.cart_new_qty_prompt", "Enter new quantity (0 removes the item): "},
            {"ch.cart_new_qty_prompt", "输入新数量（0表示移除该商品）："},
            {"jp.cart_new_qty_prompt", "新しい数量を入力してください（0でその商品を削除）："},
            {"it.cart_new_qty_prompt", "Inserisci la nuova quantità (0 rimuove l'articolo): "},
            {"fr.cart_new_qty_prompt", "Entrez la nouvelle quantité (0 supprime l'article) : "},

            {"en.cart_updated_msg", "Cart updated."},
            {"ch.cart_updated_msg", "购物车已更新。"},
            {"jp.cart_updated_msg", "カートが更新されました。"},
            {"it.cart_updated_msg", "Carrello aggiornato."},
            {"fr.cart_updated_msg", "Panier mis à jour."},

            {"en.cart_item_not_found", "Item not found in cart."},
            {"ch.cart_item_not_found", "购物车中未找到该商品。"},
            {"jp.cart_item_not_found", "カートに該当の商品が見つかりません。"},
            {"it.cart_item_not_found", "Articolo non trovato nel carrello."},
            {"fr.cart_item_not_found", "Article introuvable dans le panier."},

            {"en.cart_remove_id_prompt", "Enter watch ID to remove: "},
            {"ch.cart_remove_id_prompt", "输入要移除的手表ID："},
            {"jp.cart_remove_id_prompt", "削除する時計のIDを入力してください："},
            {"it.cart_remove_id_prompt", "Inserisci l'ID dell'orologio da rimuovere: "},
            {"fr.cart_remove_id_prompt", "Entrez l'ID de la montre à supprimer : "},

            {"en.cart_removed_msg", "Removed."},
            {"ch.cart_removed_msg", "已移除。"},
            {"jp.cart_removed_msg", "削除しました。"},
            {"it.cart_removed_msg", "Rimosso."},
            {"fr.cart_removed_msg", "Supprimé."},

            // ---- Subtotal / stock (View Cart + Checkout + product listing) ----
            {"en.cart_subtotal", "Subtotal: "},
            {"ch.cart_subtotal", "小计："},
            {"jp.cart_subtotal", "小計："},
            {"it.cart_subtotal", "Subtotale: "},
            {"fr.cart_subtotal", "Sous-total : "},

            {"en.stock_in_stock", "In stock: "},
            {"ch.stock_in_stock", "有库存："},
            {"jp.stock_in_stock", "在庫あり："},
            {"it.stock_in_stock", "Disponibile: "},
            {"fr.stock_in_stock", "En stock : "},

            {"en.stock_out_of_stock", "Out of stock"},
            {"ch.stock_out_of_stock", "缺货"},
            {"jp.stock_out_of_stock", "在庫切れ"},
            {"it.stock_out_of_stock", "Esaurito"},
            {"fr.stock_out_of_stock", "Rupture de stock"},

            // ---- Home screen ----
            {"en.home_logged_in_as", "Logged in as: "},
            {"ch.home_logged_in_as", "登录身份："},
            {"jp.home_logged_in_as", "ログイン中："},
            {"it.home_logged_in_as", "Accesso effettuato come: "},
            {"fr.home_logged_in_as", "Connecté en tant que : "},

            {"en.home_items_in_cart", "Items in cart: "},
            {"ch.home_items_in_cart", "购物车中的商品："},
            {"jp.home_items_in_cart", "カート内の商品数："},
            {"it.home_items_in_cart", "Articoli nel carrello: "},
            {"fr.home_items_in_cart", "Articles dans le panier : "},

            {"en.home_units_total", " unit(s) total)"},
            {"ch.home_units_total", " 件（总数）"},
            {"jp.home_units_total", " 個（合計）"},
            {"it.home_units_total", " unità totali)"},
            {"fr.home_units_total", " unité(s) au total)"},

            {"en.home_total_watches", "Total watches in catalog: "},
            {"ch.home_total_watches", "目录中的手表总数："},
            {"jp.home_total_watches", "カタログ内の時計総数："},
            {"it.home_total_watches", "Totale orologi nel catalogo: "},
            {"fr.home_total_watches", "Total de montres au catalogue : "},

            {"en.home_language_label", "Language: "},
            {"ch.home_language_label", "语言："},
            {"jp.home_language_label", "言語："},
            {"it.home_language_label", "Lingua: "},
            {"fr.home_language_label", "Langue : "},

            {"en.home_currency_label", "Currency: "},
            {"ch.home_currency_label", "货币："},
            {"jp.home_currency_label", "通貨："},
            {"it.home_currency_label", "Valuta: "},
            {"fr.home_currency_label", "Devise : "},

            {"en.home_tip", "Tip: use Brand / Men / Women / Other to browse, or Search to find a specific brand."},
            {"ch.home_tip", "提示：使用“品牌/男士/女士/其他”浏览商品，或使用“搜索”查找特定品牌。"},
            {"jp.home_tip", "ヒント：「ブランド／メンズ／レディース／その他」で閲覧するか、「検索」で特定のブランドを探せます。"},
            {"it.home_tip", "Suggerimento: usa Marca / Uomo / Donna / Altro per sfogliare, oppure Cerca per trovare una marca specifica."},
            {"fr.home_tip", "Astuce : utilisez Marque / Homme / Femme / Autre pour parcourir, ou Rechercher pour trouver une marque précise."},

            // ---- Admin: Manage User Accounts ----
            {"en.admin_menu_add_account", "Add new account"},
            {"ch.admin_menu_add_account", "添加新账户"},
            {"jp.admin_menu_add_account", "新しいアカウントを追加"},
            {"it.admin_menu_add_account", "Aggiungi nuovo account"},
            {"fr.admin_menu_add_account", "Ajouter un nouveau compte"},

            {"en.admin_menu_remove_account", "Remove an account"},
            {"ch.admin_menu_remove_account", "移除账户"},
            {"jp.admin_menu_remove_account", "アカウントを削除"},
            {"it.admin_menu_remove_account", "Rimuovi un account"},
            {"fr.admin_menu_remove_account", "Supprimer un compte"},

            {"en.admin_new_username", "New username: "},
            {"ch.admin_new_username", "新用户名："},
            {"jp.admin_new_username", "新しいユーザー名："},
            {"it.admin_new_username", "Nuovo nome utente: "},
            {"fr.admin_new_username", "Nouveau nom d'utilisateur : "},

            {"en.admin_new_password", "New password: "},
            {"ch.admin_new_password", "新密码："},
            {"jp.admin_new_password", "新しいパスワード："},
            {"it.admin_new_password", "Nuova password: "},
            {"fr.admin_new_password", "Nouveau mot de passe : "},

            {"en.admin_role_prompt", "Role (1 = Admin, 2 = Customer): "},
            {"ch.admin_role_prompt", "角色（1 = 管理员，2 = 顾客）："},
            {"jp.admin_role_prompt", "役割（1 = 管理者、2 = 顧客）："},
            {"it.admin_role_prompt", "Ruolo (1 = Amministratore, 2 = Cliente): "},
            {"fr.admin_role_prompt", "Rôle (1 = Administrateur, 2 = Client) : "},

            {"en.admin_account_added_prefix", "Account \""},
            {"ch.admin_account_added_prefix", "账户\""},
            {"jp.admin_account_added_prefix", "アカウント「"},
            {"it.admin_account_added_prefix", "Account \""},
            {"fr.admin_account_added_prefix", "Compte «"},

            {"en.admin_account_added_as", "\" added as "},
            {"ch.admin_account_added_as", "\"已添加为"},
            {"jp.admin_account_added_as", "」を"},
            {"it.admin_account_added_as", "\" aggiunto come "},
            {"fr.admin_account_added_as", "» ajouté en tant que "},

            {"en.admin_account_added_saved", " and saved to "},
            {"ch.admin_account_added_saved", "，并保存到"},
            {"jp.admin_account_added_saved", "として追加し、"},
            {"it.admin_account_added_saved", " e salvato in "},
            {"fr.admin_account_added_saved", " et enregistré dans "},

            {"en.admin_username_exists", "That username already exists. No changes made."},
            {"ch.admin_username_exists", "该用户名已存在。未做任何更改。"},
            {"jp.admin_username_exists", "そのユーザー名は既に存在します。変更は行われませんでした。"},
            {"it.admin_username_exists", "Quel nome utente esiste già. Nessuna modifica effettuata."},
            {"fr.admin_username_exists", "Ce nom d'utilisateur existe déjà. Aucune modification effectuée."},

            {"en.admin_remove_username_prompt", "Username to remove: "},
            {"ch.admin_remove_username_prompt", "要移除的用户名："},
            {"jp.admin_remove_username_prompt", "削除するユーザー名："},
            {"it.admin_remove_username_prompt", "Nome utente da rimuovere: "},
            {"fr.admin_remove_username_prompt", "Nom d'utilisateur à supprimer : "},

            {"en.admin_account_removed_suffix", "\" removed and "},
            {"ch.admin_account_removed_suffix", "\"已移除，"},
            {"jp.admin_account_removed_suffix", "」は削除され、"},
            {"it.admin_account_removed_suffix", "\" rimosso e "},
            {"fr.admin_account_removed_suffix", "» supprimé et "},

            {"en.admin_account_removed_updated", " updated."},
            {"ch.admin_account_removed_updated", "已更新。"},
            {"jp.admin_account_removed_updated", "が更新されました。"},
            {"it.admin_account_removed_updated", " aggiornato."},
            {"fr.admin_account_removed_updated", " mis à jour."},

            {"en.admin_remove_failed", "Could not remove account: "},
            {"ch.admin_remove_failed", "无法移除账户："},
            {"jp.admin_remove_failed", "アカウントを削除できませんでした："},
            {"it.admin_remove_failed", "Impossibile rimuovere l'account: "},
            {"fr.admin_remove_failed", "Impossible de supprimer le compte : "},

            {"en.admin_remove_reason_not_found", "No account with that username was found."},
            {"ch.admin_remove_reason_not_found", "未找到该用户名对应的账户。"},
            {"jp.admin_remove_reason_not_found", "そのユーザー名のアカウントは見つかりませんでした。"},
            {"it.admin_remove_reason_not_found", "Nessun account trovato con quel nome utente."},
            {"fr.admin_remove_reason_not_found", "Aucun compte trouvé avec ce nom d'utilisateur."},

            {"en.admin_remove_reason_last_admin", "Cannot remove the last remaining Admin account."},
            {"ch.admin_remove_reason_last_admin", "无法移除最后一个管理员账户。"},
            {"jp.admin_remove_reason_last_admin", "最後の管理者アカウントは削除できません。"},
            {"it.admin_remove_reason_last_admin", "Impossibile rimuovere l'ultimo account Amministratore rimasto."},
            {"fr.admin_remove_reason_last_admin", "Impossible de supprimer le dernier compte Administrateur restant."},

            // ---- Admin: Manage Product Inventory ----
            {"en.inv_menu_add_product", "Add new product"},
            {"ch.inv_menu_add_product", "添加新产品"},
            {"jp.inv_menu_add_product", "新しい商品を追加"},
            {"it.inv_menu_add_product", "Aggiungi nuovo prodotto"},
            {"fr.inv_menu_add_product", "Ajouter un nouveau produit"},

            {"en.inv_menu_update_product", "Update a product"},
            {"ch.inv_menu_update_product", "更新产品"},
            {"jp.inv_menu_update_product", "商品を更新"},
            {"it.inv_menu_update_product", "Aggiorna un prodotto"},
            {"fr.inv_menu_update_product", "Mettre à jour un produit"},

            {"en.inv_menu_delete_product", "Delete a product"},
            {"ch.inv_menu_delete_product", "删除产品"},
            {"jp.inv_menu_delete_product", "商品を削除"},
            {"it.inv_menu_delete_product", "Elimina un prodotto"},
            {"fr.inv_menu_delete_product", "Supprimer un produit"},

            {"en.inv_prompt_brand", "Brand: "},
            {"ch.inv_prompt_brand", "品牌："},
            {"jp.inv_prompt_brand", "ブランド："},
            {"it.inv_prompt_brand", "Marca: "},
            {"fr.inv_prompt_brand", "Marque : "},

            {"en.inv_prompt_model", "Model name: "},
            {"ch.inv_prompt_model", "型号名称："},
            {"jp.inv_prompt_model", "モデル名："},
            {"it.inv_prompt_model", "Nome del modello: "},
            {"fr.inv_prompt_model", "Nom du modèle : "},

            {"en.inv_prompt_price", "Price (USD): "},
            {"ch.inv_prompt_price", "价格（美元）："},
            {"jp.inv_prompt_price", "価格（USD）："},
            {"it.inv_prompt_price", "Prezzo (USD): "},
            {"fr.inv_prompt_price", "Prix (USD) : "},

            {"en.inv_invalid_price", "Invalid price. Enter a non-negative number: "},
            {"ch.inv_invalid_price", "价格无效。请输入非负数："},
            {"jp.inv_invalid_price", "価格が無効です。0以上の数値を入力してください："},
            {"it.inv_invalid_price", "Prezzo non valido. Inserisci un numero non negativo: "},
            {"fr.inv_invalid_price", "Prix invalide. Entrez un nombre non négatif : "},

            {"en.inv_prompt_stock", "In-stock quantity: "},
            {"ch.inv_prompt_stock", "库存数量："},
            {"jp.inv_prompt_stock", "在庫数："},
            {"it.inv_prompt_stock", "Quantità disponibile: "},
            {"fr.inv_prompt_stock", "Quantité en stock : "},

            {"en.inv_invalid_stock", "Invalid quantity. Enter a non-negative whole number: "},
            {"ch.inv_invalid_stock", "数量无效。请输入非负整数："},
            {"jp.inv_invalid_stock", "数量が無効です。0以上の整数を入力してください："},
            {"it.inv_invalid_stock", "Quantità non valida. Inserisci un numero intero non negativo: "},
            {"fr.inv_invalid_stock", "Quantité invalide. Entrez un nombre entier non négatif : "},

            {"en.inv_added_prefix", "Added new product with ID "},
            {"ch.inv_added_prefix", "已添加新产品，ID为"},
            {"jp.inv_added_prefix", "新しい商品をID"},
            {"it.inv_added_prefix", "Nuovo prodotto aggiunto con ID "},
            {"fr.inv_added_prefix", "Nouveau produit ajouté avec l'ID "},

            {"en.inv_update_id_prompt", "Enter the ID of the product to update: "},
            {"ch.inv_update_id_prompt", "输入要更新的产品ID："},
            {"jp.inv_update_id_prompt", "更新する商品のIDを入力してください："},
            {"it.inv_update_id_prompt", "Inserisci l'ID del prodotto da aggiornare: "},
            {"fr.inv_update_id_prompt", "Entrez l'ID du produit à mettre à jour : "},

            {"en.inv_no_product", "No product with that ID."},
            {"ch.inv_no_product", "没有该ID的产品。"},
            {"jp.inv_no_product", "そのIDの商品はありません。"},
            {"it.inv_no_product", "Nessun prodotto con quell'ID."},
            {"fr.inv_no_product", "Aucun produit avec cet ID."},

            {"en.inv_currently", "Currently: "},
            {"ch.inv_currently", "当前："},
            {"jp.inv_currently", "現在："},
            {"it.inv_currently", "Attualmente: "},
            {"fr.inv_currently", "Actuellement : "},

            {"en.inv_edit_brand", "Edit brand"},
            {"ch.inv_edit_brand", "编辑品牌"},
            {"jp.inv_edit_brand", "ブランドを編集"},
            {"it.inv_edit_brand", "Modifica marca"},
            {"fr.inv_edit_brand", "Modifier la marque"},

            {"en.inv_edit_name", "Edit model name"},
            {"ch.inv_edit_name", "编辑型号名称"},
            {"jp.inv_edit_name", "モデル名を編集"},
            {"it.inv_edit_name", "Modifica nome modello"},
            {"fr.inv_edit_name", "Modifier le nom du modèle"},

            {"en.inv_edit_price", "Edit price"},
            {"ch.inv_edit_price", "编辑价格"},
            {"jp.inv_edit_price", "価格を編集"},
            {"it.inv_edit_price", "Modifica prezzo"},
            {"fr.inv_edit_price", "Modifier le prix"},

            {"en.inv_edit_category", "Edit category"},
            {"ch.inv_edit_category", "编辑类别"},
            {"jp.inv_edit_category", "カテゴリーを編集"},
            {"it.inv_edit_category", "Modifica categoria"},
            {"fr.inv_edit_category", "Modifier la catégorie"},

            {"en.inv_edit_stock", "Edit stock"},
            {"ch.inv_edit_stock", "编辑库存"},
            {"jp.inv_edit_stock", "在庫を編集"},
            {"it.inv_edit_stock", "Modifica scorte"},
            {"fr.inv_edit_stock", "Modifier le stock"},

            {"en.inv_edit_done", "Done"},
            {"ch.inv_edit_done", "完成"},
            {"jp.inv_edit_done", "完了"},
            {"it.inv_edit_done", "Fatto"},
            {"fr.inv_edit_done", "Terminé"},

            {"en.inv_new_brand", "New brand: "},
            {"ch.inv_new_brand", "新品牌："},
            {"jp.inv_new_brand", "新しいブランド："},
            {"it.inv_new_brand", "Nuova marca: "},
            {"fr.inv_new_brand", "Nouvelle marque : "},

            {"en.inv_new_model", "New model name: "},
            {"ch.inv_new_model", "新型号名称："},
            {"jp.inv_new_model", "新しいモデル名："},
            {"it.inv_new_model", "Nuovo nome modello: "},
            {"fr.inv_new_model", "Nouveau nom de modèle : "},

            {"en.inv_new_price", "New price (USD): "},
            {"ch.inv_new_price", "新价格（美元）："},
            {"jp.inv_new_price", "新しい価格（USD）："},
            {"it.inv_new_price", "Nuovo prezzo (USD): "},
            {"fr.inv_new_price", "Nouveau prix (USD) : "},

            {"en.inv_new_stock", "New in-stock quantity: "},
            {"ch.inv_new_stock", "新库存数量："},
            {"jp.inv_new_stock", "新しい在庫数："},
            {"it.inv_new_stock", "Nuova quantità disponibile: "},
            {"fr.inv_new_stock", "Nouvelle quantité en stock : "},

            {"en.inv_updated_saved", "Product updated and saved to "},
            {"ch.inv_updated_saved", "产品已更新并保存到"},
            {"jp.inv_updated_saved", "商品が更新され、"},
            {"it.inv_updated_saved", "Prodotto aggiornato e salvato in "},
            {"fr.inv_updated_saved", "Produit mis à jour et enregistré dans "},

            {"en.inv_updated", "Product updated."},
            {"ch.inv_updated", "产品已更新。"},
            {"jp.inv_updated", "商品が更新されました。"},
            {"it.inv_updated", "Prodotto aggiornato."},
            {"fr.inv_updated", "Produit mis à jour."},

            {"en.inv_delete_id_prompt", "Enter the ID of the product to delete: "},
            {"ch.inv_delete_id_prompt", "输入要删除的产品ID："},
            {"jp.inv_delete_id_prompt", "削除する商品のIDを入力してください："},
            {"it.inv_delete_id_prompt", "Inserisci l'ID del prodotto da eliminare: "},
            {"fr.inv_delete_id_prompt", "Entrez l'ID du produit à supprimer : "},

            {"en.inv_deleted_saved", "Product deleted and "},
            {"ch.inv_deleted_saved", "产品已删除，"},
            {"jp.inv_deleted_saved", "商品が削除され、"},
            {"it.inv_deleted_saved", "Prodotto eliminato e "},
            {"fr.inv_deleted_saved", "Produit supprimé et "},

            {"en.inv_category_prompt", "Category (1 = Men, 2 = Women, 3 = Other): "},
            {"ch.inv_category_prompt", "类别（1 = 男士，2 = 女士，3 = 其他）："},
            {"jp.inv_category_prompt", "カテゴリー（1 = メンズ、2 = レディース、3 = その他）："},
            {"it.inv_category_prompt", "Categoria (1 = Uomo, 2 = Donna, 3 = Altro): "},
            {"fr.inv_category_prompt", "Catégorie (1 = Homme, 2 = Femme, 3 = Autre) : "},

            // ---- Cart: add/update-quantity feedback ----
            {"en.cart_qty_updated_prefix", "Updated \""},
            {"ch.cart_qty_updated_prefix", "已将\""},
            {"jp.cart_qty_updated_prefix", "「"},
            {"it.cart_qty_updated_prefix", "Aggiornato \""},
            {"fr.cart_qty_updated_prefix", "« "},

            {"en.cart_qty_updated_middle", "\" quantity to "},
            {"ch.cart_qty_updated_middle", "\"的数量更新为"},
            {"jp.cart_qty_updated_middle", "」の数量を"},
            {"it.cart_qty_updated_middle", "\": quantità aggiornata a "},
            {"fr.cart_qty_updated_middle", " » : quantité mise à jour à "},

            {"en.cart_qty_updated_suffix", " in your cart."},
            {"ch.cart_qty_updated_suffix", "（购物车内）。"},
            {"jp.cart_qty_updated_suffix", "個に更新しました。"},
            {"it.cart_qty_updated_suffix", " nel carrello."},
            {"fr.cart_qty_updated_suffix", " dans votre panier."},

            {"en.cart_added_prefix", "Added \""},
            {"ch.cart_added_prefix", "已将\""},
            {"jp.cart_added_prefix", "「"},
            {"it.cart_added_prefix", "Aggiunto \""},
            {"fr.cart_added_prefix", "« "},

            {"en.cart_added_middle", "\" (x"},
            {"ch.cart_added_middle", "\"（x"},
            {"jp.cart_added_middle", "」（x"},
            {"it.cart_added_middle", "\" (x"},
            {"fr.cart_added_middle", " » (x"},

            {"en.cart_added_suffix", ") to your cart."},
            {"ch.cart_added_suffix", "）加入购物车。"},
            {"jp.cart_added_suffix", "）をカートに追加しました。"},
            {"it.cart_added_suffix", ") aggiunto al carrello."},
            {"fr.cart_added_suffix", ") ajouté à votre panier."},

            {"en.cart_qty_label", "Qty: "},
            {"ch.cart_qty_label", "数量："},
            {"jp.cart_qty_label", "数量："},
            {"it.cart_qty_label", "Quantità: "},
            {"fr.cart_qty_label", "Qté : "},

            {"en.no_watch_shown_here", "No watch with that ID was shown here."},
            {"ch.no_watch_shown_here", "此处未显示该ID的手表。"},
            {"jp.no_watch_shown_here", "そのIDの時計はここには表示されていません。"},
            {"it.no_watch_shown_here", "Nessun orologio con quell'ID mostrato qui."},
            {"fr.no_watch_shown_here", "Aucune montre avec cet ID n'a été affichée ici."},

            // ---- Brand menu ----
            {"en.brand_all_watches", "All Watches"},
            {"ch.brand_all_watches", "所有手表"},
            {"jp.brand_all_watches", "すべての時計"},
            {"it.brand_all_watches", "Tutti gli Orologi"},
            {"fr.brand_all_watches", "Toutes les Montres"},

            // ---- Search ----
            {"en.search_brand_prompt", "Enter brand keyword: "},
            {"ch.search_brand_prompt", "输入品牌关键词："},
            {"jp.search_brand_prompt", "ブランドのキーワードを入力してください："},
            {"it.search_brand_prompt", "Inserisci una parola chiave per la marca: "},
            {"fr.search_brand_prompt", "Entrez un mot-clé de marque : "},

            {"en.search_found_prefix", "Found "},
            {"ch.search_found_prefix", "找到"},
            {"jp.search_found_prefix", "検索結果："},
            {"it.search_found_prefix", "Trovati "},
            {"fr.search_found_prefix", "Trouvé "},

            {"en.search_found_suffix", " result(s)."},
            {"ch.search_found_suffix", "个结果。"},
            {"jp.search_found_suffix", "件"},
            {"it.search_found_suffix", " risultato/i."},
            {"fr.search_found_suffix", " résultat(s)."},

            // ---- Shipping address display ----
            {"en.shipping_display_recipient", "Recipient: "},
            {"ch.shipping_display_recipient", "收件人："},
            {"jp.shipping_display_recipient", "受取人："},
            {"it.shipping_display_recipient", "Destinatario: "},
            {"fr.shipping_display_recipient", "Destinataire : "},

            {"en.shipping_display_phone", "Phone:     "},
            {"ch.shipping_display_phone", "电话：    "},
            {"jp.shipping_display_phone", "電話：    "},
            {"it.shipping_display_phone", "Telefono:  "},
            {"fr.shipping_display_phone", "Téléphone :"},

            {"en.shipping_display_address", "Address:   "},
            {"ch.shipping_display_address", "地址：    "},
            {"jp.shipping_display_address", "住所：    "},
            {"it.shipping_display_address", "Indirizzo: "},
            {"fr.shipping_display_address", "Adresse :  "},

            // ---- Payment method selection ----
            {"en.payment_select_method_title", "Select a payment method:"},
            {"ch.payment_select_method_title", "选择支付方式："},
            {"jp.payment_select_method_title", "支払い方法を選択してください："},
            {"it.payment_select_method_title", "Seleziona un metodo di pagamento:"},
            {"fr.payment_select_method_title", "Choisissez un mode de paiement :"},

            {"en.payment_invalid_choice", "Invalid choice. Please enter 1-4."},
            {"ch.payment_invalid_choice", "选择无效。请输入1-4。"},
            {"jp.payment_invalid_choice", "選択が無効です。1〜4を入力してください。"},
            {"it.payment_invalid_choice", "Scelta non valida. Inserisci un numero da 1 a 4."},
            {"fr.payment_invalid_choice", "Choix invalide. Veuillez entrer un nombre entre 1 et 4."},

            {"en.payment_processing_prefix", "Processing "},
            {"ch.payment_processing_prefix", "正在处理"},
            {"jp.payment_processing_prefix", "処理中："},
            {"it.payment_processing_prefix", "Elaborazione di "},
            {"fr.payment_processing_prefix", "Traitement de "},

            {"en.payment_processing_via", " via "},
            {"ch.payment_processing_via", "，支付方式："},
            {"jp.payment_processing_via", "（支払い方法："},
            {"it.payment_processing_via", " tramite "},
            {"fr.payment_processing_via", " via "},

            {"en.payment_success", "Payment successful. Thank you for your purchase!"},
            {"ch.payment_success", "支付成功。感谢您的购买！"},
            {"jp.payment_success", "お支払いが完了しました。ご購入ありがとうございます！"},
            {"it.payment_success", "Pagamento riuscito. Grazie per il tuo acquisto!"},
            {"fr.payment_success", "Paiement réussi. Merci pour votre achat !"},

            // ---- Generic input validation ----
            {"en.invalid_number_prefix", "Invalid input. Please enter a number between "},
            {"ch.invalid_number_prefix", "输入无效。请输入一个介于"},
            {"jp.invalid_number_prefix", "入力が無効です。"},
            {"it.invalid_number_prefix", "Input non valido. Inserisci un numero compreso tra "},
            {"fr.invalid_number_prefix", "Entrée invalide. Veuillez entrer un nombre entre "},

            {"en.invalid_number_and", " and "},
            {"ch.invalid_number_and", "和"},
            {"jp.invalid_number_and", "から"},
            {"it.invalid_number_and", " e "},
            {"fr.invalid_number_and", " et "},

            {"en.invalid_number_suffix", "."},
            {"ch.invalid_number_suffix", "之间的数字。"},
            {"jp.invalid_number_suffix", "の間の数値を入力してください。"},
            {"it.invalid_number_suffix", "."},
            {"fr.invalid_number_suffix", "."},

            // ---- Admin: low-stock alert ----
            {"en.low_stock_alert_title", "*** ADMIN ALERT: LOW STOCK WARNING ***"},
            {"ch.low_stock_alert_title", "***管理员警报：库存不足警告***"},
            {"jp.low_stock_alert_title", "***管理者アラート：在庫不足警告***"},
            {"it.low_stock_alert_title", "*** AVVISO AMMINISTRATORE: SCORTE BASSE ***"},
            {"fr.low_stock_alert_title", "*** ALERTE ADMIN : STOCK FAIBLE ***"},

            {"en.low_stock_alert_intro", "The following item(s) need to be restocked:"},
            {"ch.low_stock_alert_intro", "以下商品需要补货："},
            {"jp.low_stock_alert_intro", "以下の商品は補充が必要です："},
            {"it.low_stock_alert_intro", "I seguenti articoli devono essere riforniti:"},
            {"fr.low_stock_alert_intro", "Les articles suivants doivent être réapprovisionnés :"},

            {"en.low_stock_alert_item_suffix", " unit(s) left in stock."},
            {"ch.low_stock_alert_item_suffix", "件库存。"},
            {"jp.low_stock_alert_item_suffix", "個の在庫のみです。"},
            {"it.low_stock_alert_item_suffix", " unità rimaste in stock."},
            {"fr.low_stock_alert_item_suffix", " unité(s) restante(s) en stock."},

            {"en.low_stock_alert_only", " -- only "},
            {"ch.low_stock_alert_only", " —— 仅剩"},
            {"jp.low_stock_alert_only", " ―― 残り"},
            {"it.low_stock_alert_only", " -- solo "},
            {"fr.low_stock_alert_only", " -- seulement "},
        };
        return table;
    }
};

// Holds the two cross-cutting preferences (Language, Currency) for the
// current session. Selecting a language automatically re-maps the
// currency to that region's default (per project requirement 2). The
// currency can also be set directly (used for MMK, which has no
// associated UI language in this app).
class Settings {
public:
    Settings() : language(Language::English), currency(currencyForLanguage(Language::English)) {}

    Language getLanguage() const { return language; }
    Currency getCurrency() const { return currency; }

    // Changing the language always re-derives the currency; there is no
    // way to set one without the other, matching the "auto-update" rule.
    void setLanguage(Language lang) {
        language = lang;
        currency = currencyForLanguage(lang);
    }

    // Direct currency override (e.g. selecting MMK independently of language).
    void setCurrency(Currency c) { currency = c; }

    // Convenience passthrough so call sites don't need to import Localization directly.
    std::string label(const std::string& key) const {
        return Localization::text(key, language);
    }

    // Converts a USD catalog price into the active currency.
    double convert(double usdPrice) const {
        return usdPrice * exchangeRateFromUSD(currency);
    }

    // Formats a USD catalog price in the active currency. Most currencies
    // are rendered as a currency-symbol-prefixed string (e.g. "$799.00",
    // "\xC2\xA5115199", "\xE2\x82\xAC743.07"). JPY and MMK conventionally
    // have no minor unit, so both are rendered with 0 decimals. MMK is the
    // one exception to the symbol-prefix convention: per Myanmar
    // mobile-banking checkout conventions it is rendered as the plain,
    // comma-grouped numeric value followed by a space and "MMK" (e.g.
    // "1,678,798 MMK") rather than with a leading "K" symbol.
    std::string formatPrice(double usdPrice) const {
        double converted = convert(usdPrice);

        if (currency == Currency::MMK) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(0) << converted;
            return groupThousands(oss.str()) + " MMK";
        }

        std::ostringstream oss;
        oss << currencySymbol(currency) << std::fixed;
        if (currency == Currency::JPY) {
            oss << std::setprecision(0);
        } else {
            oss << std::setprecision(2);
        }
        oss << converted;
        return oss.str();
    }

private:
    // Inserts thousands separators into a non-negative, sign-free integer
    // string (e.g. "1678798" -> "1,678,798"). Used to comma-format the
    // whole-number MMK amount in formatPrice().
    static std::string groupThousands(const std::string& digits) {
        std::string result;
        int count = 0;
        for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
            if (count != 0 && count % 3 == 0) {
                result.push_back(',');
            }
            result.push_back(*it);
            ++count;
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    Language language;
    Currency currency;
};

// ============================ 1.6 FILE STORAGE ==============================
// Small shared helpers for the two persisted text files (users.txt,
// inventory.txt). Both files use '|' as a field separator since none of
// the stored fields (names, brands, addresses) are expected to contain it.

namespace Storage {

const std::string USERS_FILE = "users.txt";
const std::string INVENTORY_FILE = "inventory.txt";

// Splits a line on '|' into exactly the expected number of fields.
// Returns an empty vector if the line doesn't split cleanly, so callers
// can skip malformed/blank lines instead of crashing on a corrupt file.
std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, '|')) {
        fields.push_back(field);
    }
    return fields;
}

} // namespace Storage

// ============================== 2. USER =====================================

// Represents a system account. Accounts now carry a Role (Admin/Customer)
// used for role-based access control. The whitelist is no longer held
// only in memory: it is loaded from, and persisted to, users.txt so an
// Admin's added/removed accounts survive across program runs.
class User {
public:
    User(const std::string& username, const std::string& password, Role role = Role::Customer)
        : username(username), password(password), role(role) {}

    const std::string& getUsername() const { return username; }
    const std::string& getPassword() const { return password; }
    Role getRole() const { return role; }

    // Checks the given credentials against the current whitelist. Returns
    // true only on an exact username+password match.
    static bool authenticate(const std::string& username, const std::string& password) {
        return findUser(username, password) != nullptr;
    }

    // Same lookup as authenticate(), but also hands back the matched
    // account (so the caller can read its Role).
    static const User* findUser(const std::string& username, const std::string& password) {
        for (const auto& u : accounts()) {
            if (u.username == username && u.password == password) {
                return &u;
            }
        }
        return nullptr;
    }

    static bool userExists(const std::string& username) {
        for (const auto& u : accounts()) {
            if (u.username == username) {
                return true;
            }
        }
        return false;
    }

    // Admin Management Feature: add a new account to the whitelist and
    // persist the whole account list to users.txt immediately.
    // Returns false if the username is already taken.
    static bool addUser(const std::string& username, const std::string& password, Role role) {
        if (userExists(username)) {
            return false;
        }
        accounts().emplace_back(username, password, role);
        saveToFile();
        return true;
    }

    // Admin Management Feature: remove an account from the whitelist and
    // persist the change to users.txt. Refuses to remove an unknown
    // account, or the last remaining Admin (the store must always keep
    // at least one Admin able to log in).
    static bool removeUser(const std::string& username, std::string& reasonIfFailed) {
        auto& accts = accounts();
        auto it = std::find_if(accts.begin(), accts.end(),
                                [&](const User& u) { return u.username == username; });
        if (it == accts.end()) {
            reasonIfFailed = "admin_remove_reason_not_found";
            return false;
        }
        if (it->role == Role::Admin && countAdmins() <= 1) {
            reasonIfFailed = "admin_remove_reason_last_admin";
            return false;
        }
        accts.erase(it);
        saveToFile();
        return true;
    }

    static int countAdmins() {
        int count = 0;
        for (const auto& u : accounts()) {
            if (u.role == Role::Admin) {
                ++count;
            }
        }
        return count;
    }

    // Read-only snapshot of every account, used to render the user
    // management screen.
    static const std::vector<User>& listUsers() { return accounts(); }

    // Rewrites users.txt from the current in-memory account list. Called
    // automatically after every add/remove, but exposed publicly in case
    // the caller wants to force a re-save.
    static void saveToFile() {
        std::ofstream out(Storage::USERS_FILE, std::ios::trunc);
        if (!out) {
            std::cerr << "Warning: could not write " << Storage::USERS_FILE << "\n";
            return;
        }
        for (const auto& u : accounts()) {
            out << u.username << "|" << u.password << "|" << roleToString(u.role) << "\n";
        }
    }

private:
    std::string username;
    std::string password;
    Role role;

    // Loads accounts from users.txt. If the file doesn't exist yet (first
    // run), seeds the original 5 default accounts and writes the file so
    // subsequent runs read the same data back from disk.
    static std::vector<User> loadFromFile() {
        std::vector<User> loaded;
        std::ifstream in(Storage::USERS_FILE);
        if (!in) {
            return loaded; // signals "file missing" to the caller
        }
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto fields = Storage::splitFields(line);
            if (fields.size() < 3) continue; // skip malformed lines
            loaded.emplace_back(fields[0], fields[1], roleFromString(fields[2]));
        }
        return loaded;
    }

    // Function-local static: guarantees the vector is constructed exactly
    // once, on first use, regardless of global initialization order. It is
    // intentionally mutable (not const) so Admins can add/remove accounts
    // while the app is running. On first-ever run (no users.txt present)
    // the original 5 default accounts are seeded and immediately persisted.
    static std::vector<User>& accounts() {
        static std::vector<User> accts = [] {
            std::vector<User> fromFile = loadFromFile();
            if (!fromFile.empty()) {
                return fromFile;
            }
            std::vector<User> defaults = {
                User("Chris",           "123456", Role::Admin),
                User("MyatMinKoKo",     "123456", Role::Customer),
                User("NaingMyoChit",    "123456", Role::Customer),
                User("SuLattKhinKhin",  "123456", Role::Customer),
                User("Watch",           "123456", Role::Customer)   // 5th account added to satisfy "5 specific users"
            };
            return defaults;
        }();
        static bool seeded = false;
        if (!seeded) {
            seeded = true;
            // If users.txt didn't exist, this writes it for the first time.
            // If it did exist, this is a harmless no-op re-save.
            std::ifstream check(Storage::USERS_FILE);
            if (!check) {
                saveToFile();
            }
        }
        return accts;
    }
};

// ============================== 3. WATCH ====================================

// Plain data class representing a single watch product in the store.
// `stock` (in-stock quantity) was added so the Admin can track inventory
// levels, per the file-based inventory management requirement.
class Watch {
public:
    Watch(int id, std::string brand, std::string name, double price, Category category, int stock = 0)
        : id(id), brand(std::move(brand)), name(std::move(name)),
          price(price), category(category), stock(stock) {}

    int getId() const { return id; }
    const std::string& getBrand() const { return brand; }
    const std::string& getName() const { return name; }
    double getPrice() const { return price; }
    Category getCategory() const { return category; }
    int getStock() const { return stock; }

    // Setters used by the Admin's "Update product" flow.
    void setBrand(const std::string& newBrand) { brand = newBrand; }
    void setName(const std::string& newName) { name = newName; }
    void setPrice(double newPrice) { price = newPrice; }
    void setCategory(Category newCategory) { category = newCategory; }
    void setStock(int newStock) { stock = newStock; }

    // Prints a single formatted line: [id] Brand  Name  price (Category)  Stock: n
    // Price and category both adapt to the active Settings (currency + language).
    void display(const Settings& settings) const {
        std::cout << "[" << std::right << std::setw(2) << id << "] "
                  << std::left  << std::setw(20) << brand
                  << std::left  << std::setw(24) << name
                  << std::right << std::setw(12) << settings.formatPrice(price)
                  << "  (" << settings.label(categoryLocalizationKey(category)) << ")"
                  << "  " << (stock > 0 ? (settings.label("stock_in_stock") + std::to_string(stock)) : settings.label("stock_out_of_stock"))
                  << "\n";
    }

private:
    int id;
    std::string brand;
    std::string name;
    double price;
    Category category;
    int stock;
};

// ============================ 4. WATCHCATALOG ===============================

// Owns the master inventory of watches. In addition to the original
// read-only browsing/search operations, the catalog now also supports the
// mutating operations an Admin needs: add, update, and delete a product.
// Inventory is loaded from, and persisted to, inventory.txt so Admin
// changes survive across program runs.
class WatchCatalog {
public:
    WatchCatalog() { loadFromFileOrSeed(); }

    // All watches whose category matches (used by Men/Women/Other menus).
    std::vector<Watch> getByCategory(Category category) const {
        std::vector<Watch> result;
        for (const auto& w : watches) {
            if (w.getCategory() == category) {
                result.push_back(w);
            }
        }
        return result;
    }

    // Every watch in the store (used by the "Brand" browse view and by the
    // Admin's inventory management screen).
    const std::vector<Watch>& getAll() const { return watches; }

    // Case-insensitive substring match against brand name.
    std::vector<Watch> searchByBrand(const std::string& keyword) const {
        std::vector<Watch> result;
        const std::string needle = toLower(keyword);
        for (const auto& w : watches) {
            if (toLower(w.getBrand()).find(needle) != std::string::npos) {
                result.push_back(w);
            }
        }
        return result;
    }

    // Looks up one watch by id; returns nullptr if not found.
    const Watch* findById(int id) const {
        for (const auto& w : watches) {
            if (w.getId() == id) {
                return &w;
            }
        }
        return nullptr;
    }

    // Mutable lookup, used by the Admin's "Update product" flow.
    Watch* findByIdMutable(int id) {
        for (auto& w : watches) {
            if (w.getId() == id) {
                return &w;
            }
        }
        return nullptr;
    }

    // Admin Management Feature: add a brand-new product to the catalog and
    // persist the change to inventory.txt. IDs are assigned automatically
    // so they never collide with existing ones.
    int addWatch(const std::string& brand, const std::string& name, double price, Category category, int stock) {
        int newId = nextId++;
        watches.emplace_back(newId, brand, name, price, category, stock);
        saveToFile();
        return newId;
    }

    // Checkout Feature: deducts `quantity` units from the stock of the
    // product identified by `id`. Stock is clamped at 0 (never goes
    // negative) as a defensive measure. Returns false if the id isn't
    // found. Does NOT persist by itself -- callers that deduct multiple
    // items in a single checkout should call saveToFile() once afterward
    // (see MenuManager::handleCheckout).
    bool deductStock(int id, int quantity) {
        Watch* found = findByIdMutable(id);
        if (!found) {
            return false;
        }
        int newStock = found->getStock() - quantity;
        found->setStock(newStock < 0 ? 0 : newStock);
        return true;
    }

    // Admin Management Feature: remove a product from the catalog and
    // persist the change to inventory.txt.
    bool deleteWatch(int id) {
        auto it = std::find_if(watches.begin(), watches.end(),
                                [id](const Watch& w) { return w.getId() == id; });
        if (it == watches.end()) {
            return false;
        }
        watches.erase(it);
        saveToFile();
        return true;
    }

    // Admin Management Feature: persist the catalog to inventory.txt.
    // Called automatically after add/delete, and should also be called
    // by the caller after any in-place edit via findByIdMutable() (the
    // Admin "Update product" flow does this once editing is finished).
    void saveToFile() const {
        std::ofstream out(Storage::INVENTORY_FILE, std::ios::trunc);
        if (!out) {
            std::cerr << "Warning: could not write " << Storage::INVENTORY_FILE << "\n";
            return;
        }
        for (const auto& w : watches) {
            out << w.getId() << "|" << w.getBrand() << "|" << w.getName() << "|"
                << std::fixed << std::setprecision(2) << w.getPrice() << "|"
                << categoryToString(w.getCategory()) << "|" << w.getStock() << "\n";
        }
    }

private:
    std::vector<Watch> watches;
    int nextId = 16; // seedData() hard-codes ids 1-15, so new products start at 16.

    // Loads inventory from inventory.txt. If the file doesn't exist yet
    // (first run), seeds the original sample inventory and writes the
    // file so subsequent runs read the same data back from disk.
    void loadFromFileOrSeed() {
        std::ifstream in(Storage::INVENTORY_FILE);
        if (!in) {
            seedData();
            saveToFile();
            return;
        }

        std::string line;
        int maxId = 0;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto fields = Storage::splitFields(line);
            if (fields.size() < 6) continue; // skip malformed lines
            try {
                int id = std::stoi(fields[0]);
                const std::string& brand = fields[1];
                const std::string& name = fields[2];
                double price = std::stod(fields[3]);
                Category category = categoryFromString(fields[4]);
                int stock = std::stoi(fields[5]);
                watches.emplace_back(id, brand, name, price, category, stock);
                maxId = std::max(maxId, id);
            } catch (const std::exception&) {
                continue; // skip lines with unparsable numeric fields
            }
        }

        if (watches.empty()) {
            // File existed but was empty/corrupt: fall back to seed data.
            seedData();
            saveToFile();
        } else {
            nextId = maxId + 1;
        }
    }

    // Populates the original sample inventory (15 watches) with starting
    // stock levels. Used only the very first time the app runs, before
    // inventory.txt exists.
    void seedData() {
        // ----- Men (4 items) -----
        watches.emplace_back(1, "Rolex",         "Submariner",           8999.99, Category::Men, 3);
        watches.emplace_back(2, "Omega",         "Speedmaster",          5999.00, Category::Men, 5);
        watches.emplace_back(3, "Hugo Boss",     "Ocean Chronograph",     249.00, Category::Men, 20);
        watches.emplace_back(4, "Dennis Martin", "Heritage Chronograph",  199.00, Category::Men, 15);

        // ----- Women (4 items) -----
        watches.emplace_back(5, "Cartier",             "Tank Must",         3999.00, Category::Women, 4);
        watches.emplace_back(6, "Alexander Christie",  "Ladies Diamond",     159.00, Category::Women, 18);
        watches.emplace_back(7, "Fossil",              "Carlie",             129.00, Category::Women, 25);
        watches.emplace_back(8, "Citizen",             "Silhouette Crystal", 249.00, Category::Women, 12);

        // ----- Other / Smart & Tech (7 items) -----
        watches.emplace_back(9, "Apple Watch",        "Ultra 2",                  799.00, Category::Other, 10);
        watches.emplace_back(10, "Samsung",            "Galaxy Ring Titanium",     399.00, Category::Other, 14);
        watches.emplace_back(11, "Oura",               "Oura Ring Gen 4",          349.00, Category::Other, 9);
        watches.emplace_back(12, "Ray-Ban",            "Meta Smart Glasses",       299.00, Category::Other, 11);
        watches.emplace_back(13, "Humane",             "AI Pin Lunar Edition",     499.00, Category::Other, 6);
        watches.emplace_back(14, "Fossil",             "Gen 6 Smartwatch",         229.00, Category::Other, 17);
        watches.emplace_back(15, "Garmin",             "Fenix 8 Sapphire Solar",   999.00, Category::Other, 8);
    }

    // Local helper: lowercases a string for case-insensitive comparisons.
    static std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }
};

// ================================ 5. CART ===================================

// One line item in the cart: a watch plus how many the customer wants.
struct CartItem {
    Watch watch;
    int quantity;
    CartItem(const Watch& watch, int quantity) : watch(watch), quantity(quantity) {}
};

// Holds the watches the current user intends to buy, now with per-item
// quantities so a customer can buy more than one of the same watch
// (project requirement 3) instead of only ever adding a single unit.
class Cart {
public:
    // Adds `quantity` units of `watch`. If the watch is already in the
    // cart, its quantity is increased rather than creating a duplicate line.
    void addItem(const Watch& watch, int quantity, const Settings& settings) {
        for (auto& item : items) {
            if (item.watch.getId() == watch.getId()) {
                item.quantity += quantity;
                std::cout << settings.label("cart_qty_updated_prefix") << watch.getName()
                          << settings.label("cart_qty_updated_middle") << item.quantity
                          << settings.label("cart_qty_updated_suffix") << "\n";
                return;
            }
        }
        items.emplace_back(watch, quantity);
        std::cout << settings.label("cart_added_prefix") << watch.getName()
                  << settings.label("cart_added_middle") << quantity
                  << settings.label("cart_added_suffix") << "\n";
    }

    // Removes the entire line item whose id matches. Returns false if not found.
    bool removeItem(int watchId) {
        auto it = std::find_if(items.begin(), items.end(),
                                [watchId](const CartItem& c) { return c.watch.getId() == watchId; });
        if (it == items.end()) {
            return false;
        }
        items.erase(it);
        return true;
    }

    // Directly sets the quantity for an existing line item. Setting the
    // quantity to 0 (or below) removes the item entirely. Returns false if
    // the item isn't in the cart.
    bool setQuantity(int watchId, int newQuantity) {
        auto it = std::find_if(items.begin(), items.end(),
                                [watchId](const CartItem& c) { return c.watch.getId() == watchId; });
        if (it == items.end()) {
            return false;
        }
        if (newQuantity <= 0) {
            items.erase(it);
        } else {
            it->quantity = newQuantity;
        }
        return true;
    }

    void clear() { items.clear(); }

    double getTotal() const {
        double total = 0.0;
        for (const auto& item : items) {
            total += item.watch.getPrice() * item.quantity;
        }
        return total;
    }

    // Total number of individual units across all line items (distinct
    // from items.size(), which only counts distinct products).
    int getTotalUnitCount() const {
        int count = 0;
        for (const auto& item : items) {
            count += item.quantity;
        }
        return count;
    }

    const std::vector<CartItem>& getItems() const { return items; }
    bool isEmpty() const { return items.empty(); }

    // Prints every item (with quantity + subtotal) plus the running total,
    // in the active language/currency.
    void display(const Settings& settings) const {
        if (items.empty()) {
            std::cout << settings.label("cart_empty") << "\n";
            return;
        }
        std::cout << "\n" << settings.label("cart_title") << "\n";
        for (const auto& item : items) {
            item.watch.display(settings);
            std::cout << "      " << settings.label("cart_qty_label") << item.quantity
                      << "   " << settings.label("cart_subtotal") << settings.formatPrice(item.watch.getPrice() * item.quantity) << "\n";
        }
        std::cout << "-----------------\n";
        std::cout << settings.label("cart_total") << settings.formatPrice(getTotal()) << "\n";
    }

private:
    std::vector<CartItem> items;
};

// =========================== 5.5 SHIPPING ADDRESS ===========================

// Holds the delivery details a customer supplies during checkout: who the
// order should be delivered to, a contact phone number, and the physical
// shipping address/city.
struct ShippingAddress {
    std::string recipientName;
    std::string phone;
    std::string addressLine; // street/city; kept as one free-text field for simplicity

    bool isEmpty() const {
        return recipientName.empty() && phone.empty() && addressLine.empty();
    }

    void display(const Settings& settings) const {
        std::cout << settings.label("shipping_display_recipient") << recipientName << "\n"
                  << settings.label("shipping_display_phone") << phone << "\n"
                  << settings.label("shipping_display_address") << addressLine << "\n";
    }
};

// =============================== 6. PAYMENT =================================

// Encapsulates the checkout/payment step: choosing a Myanmar mobile-banking
// method and "processing" the payment for a given amount.
class Payment {
public:
    // Displays the 4 supported Myanmar mobile-banking payment methods and
    // returns the user's choice.
    static PaymentMethod selectMethod(const Settings& settings) {
        std::cout << "\n" << settings.label("payment_select_method_title") << "\n"
                  << "1. KPay (KBZPay)\n2. AYA Pay\n3. CB Pay\n4. Wave Pay\n5. PayPal\n6. Payoneer\n7. Google Pay (GPay)\n8. Apple Pay\n";

        int choice = 0;
        while (true) {
            std::cout << "> ";
            if (std::cin >> choice && choice >= 1 && choice <= 4) {
                break;
            }
            std::cout << settings.label("payment_invalid_choice") << "\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        switch (choice) {
            case 1: return PaymentMethod::KPay;
            case 2: return PaymentMethod::AyaPay;
            case 3: return PaymentMethod::CBPay;
            case 4: return PaymentMethod::WavePay;
            case 5: return PaymentMethod::PayPal;
            case 6: return PaymentMethod::Payoneer;
            case 7: return PaymentMethod::GPay;
            case 8: return PaymentMethod::ApplePay;
            
        }
    }

    // Simulates submitting the payment to a gateway. Returns true on
    // success (always succeeds here; a real integration would call an
    // external API and could fail).
    static bool process(PaymentMethod method, double amount, const Settings& settings) {
        std::cout << settings.label("payment_processing_prefix") << settings.formatPrice(amount)
                  << settings.label("payment_processing_via") << methodToString(method) << "...\n";
        // NOTE: this is a simulated transaction. A production build would
        // call the real KBZPay/AYA Pay/CB Pay/Wave Pay SDK or REST API
        // here and return the gateway's actual success/failure result.
        std::cout << settings.label("payment_success") << "\n";
        return true;
    }

    static std::string methodToString(PaymentMethod method) {
        switch (method) {
            case PaymentMethod::KPay:    return "KPay (KBZPay)";
            case PaymentMethod::AyaPay:  return "AYA Pay";
            case PaymentMethod::CBPay:   return "CB Pay";
            case PaymentMethod::WavePay: return "Wave Pay";
            case PaymentMethod::PayPal:  return "Paypal";
            case PaymentMethod::Payoneer:return "Payoneer";
            case PaymentMethod::GPay:    return "Gpay";
            case PaymentMethod::ApplePay:return "ApplePay";
        }
        return "Unknown";
    }
};

// ============================= 7. MENUMANAGER ===============================

// Orchestrates the entire console application: login, the main menu,
// every sub-menu (Home/Brand/Men/Women/Other/Search/Cart/Checkout/Settings),
// and, for Admins only, the Admin Panel (user & inventory management).
// This is the only class that talks directly to std::cin/std::cout for
// navigation; the other classes stay UI-light.
class MenuManager {
public:
    // Admin Stock-Refill Notification: any purchased item whose stock
    // drops below this quantity after a checkout triggers a low-stock
    // alert on the console (see notifyLowStock()).
    static const int LOW_STOCK_THRESHOLD = 10;

    MenuManager() : catalog(), cart(), settings(), currentUser(""), currentRole(Role::Customer) {}

    // Entry point called from main(). Blocks until the user exits.
    void run() {
        if (!login()) {
            std::cout << settings.label("too_many_attempts") << "\n";
            return;
        }

        bool running = true;
        while (running) {
            // Admins see one extra menu entry ("Admin Panel") wedged in
            // between Settings and Exit. Regular customers never see it and
            // have no way to reach handleAdminPanel() at all -- this is the
            // enforcement point for "customers must be strictly restricted
            // from admin-level functionality".
            const bool isAdmin = (currentRole == Role::Admin);
            const int adminChoiceNum = 11;                 // only valid when isAdmin
            const int exitChoiceNum = isAdmin ? 12 : 11;

            std::cout << "\n" << settings.label("main_menu_title") << "\n"
                      << "1. " << settings.label("menu_home") << "\n"
                      << "2. " << settings.label("menu_brand") << "\n"
                      << "3. " << settings.label("menu_men") << "\n"
                      << "4. " << settings.label("menu_women") << "\n"
                      << "5. " << settings.label("menu_other") << "\n"
                      << "6. " << settings.label("menu_search") << "\n"
                      << "7. " << settings.label("menu_cart") << "\n"
                      << "8. " << settings.label("menu_checkout") << "\n"
                      << "9. " << settings.label("menu_settings") << "\n"
                      << "10. " << settings.label("menu_currency") << "\n";
            if (isAdmin) {
                std::cout << "11. " << settings.label("menu_admin") << "\n"
                          << "12. " << settings.label("menu_exit") << "\n";
            } else {
                std::cout << "11. " << settings.label("menu_exit") << "\n";
            }

            int choice = readIntChoice(1, exitChoiceNum);

            switch (choice) {
                case 1: showMainMenu(); break;
                case 2: handleBrandMenu(); break;
                case 3: handleCategoryMenu(Category::Men,   settings.label("menu_men")); break;
                case 4: handleCategoryMenu(Category::Women, settings.label("menu_women")); break;
                case 5: handleCategoryMenu(Category::Other, settings.label("menu_other")); break;
                case 6: handleSearch(); break;
                case 7: handleCart(); break;
                case 8: handleCheckout(); break;
                case 9: handleSettingsMenu(); break;
                case 10: handleCurrencyMenu(); break;
                default:
                    if (isAdmin && choice == adminChoiceNum) {
                        handleAdminPanel();
                    } else {
                        // Either "11" for a non-admin, or "12" for an admin: both are Exit.
                        std::cout << settings.label("goodbye") << currentUser << settings.label("goodbye_suffix") << "\n";
                        running = false;
                    }
                    break;
                }
        }
    }

private:
    WatchCatalog catalog;
    Cart cart;
    Settings settings;
    std::string currentUser;
    Role currentRole;
    ShippingAddress lastShippingAddress; // remembered for convenience across checkouts

    // Centralized, validated integer input. Every menu in the app funnels
    // through this so bad input (letters, out-of-range numbers) can never
    // crash the program or desync std::cin.
    int readIntChoice(int min, int max) {
        int choice;
        while (true) {
            std::cout << "> ";
            if (std::cin >> choice && choice >= min && choice <= max) {
                return choice;
            }
            std::cout << settings.label("invalid_number_prefix")
                      << min << settings.label("invalid_number_and") << max
                      << settings.label("invalid_number_suffix") << "\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    // Reads a positive integer quantity, defaulting to 1 on blank/invalid
    // input so the existing "just press enter" style flows still work.
    static int readQuantity() {
        int qty;
        if (!(std::cin >> qty) || qty <= 0) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return 1;
        }
        return qty;
    }

    // Discards any leftover newline sitting in the input buffer after a
    // prior `std::cin >> x` extraction. Must be called exactly once,
    // right before the *first* std::getline in a sequence -- calling it
    // again before subsequent getlines would incorrectly swallow the next
    // full line instead of just a stray newline.
    static void primeForLineInput() {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Reads one full line of free-text input (used for shipping address
    // fields, which may contain spaces). Callers must call
    // primeForLineInput() once before the first readLine() in a sequence.
    static std::string readLine() {
        std::string line;
        std::getline(std::cin, line);
        return line;
    }

    // Prompts for username/password up to 3 times and checks them via
    // User::authenticate(). Only whitelisted accounts can pass; the
    // matched account's Role determines whether the Admin Panel unlocks.
    bool login() {
        std::cout << settings.label("login_title") << "\n";
        for (int attempt = 0; attempt < 3; ++attempt) {
            std::string username, password;
            std::cout << settings.label("username_prompt");
            std::cin >> username;
            std::cout << settings.label("password_prompt");
            std::cin >> password;

            const User* matched = User::findUser(username, password);
            if (matched) {
                currentUser = username;
                currentRole = matched->getRole();
                std::cout << settings.label("welcome") << username
                          << " (" << roleToString(currentRole) << ")!\n";
                return true;
            }
            std::cout << settings.label("invalid_credentials") << (2 - attempt) << "\n";
        }
        return false;
    }

    // "Home" sub-menu: a lightweight dashboard showing who is logged in
    // and a quick snapshot of cart/catalog size.
    void showMainMenu() {
        std::cout << "\n--- " << settings.label("menu_home") << " ---\n"
                  << settings.label("home_logged_in_as") << currentUser << " (" << roleToString(currentRole) << ")\n"
                  << settings.label("home_items_in_cart") << cart.getItems().size()
                  << " (" << cart.getTotalUnitCount() << settings.label("home_units_total") << "\n"
                  << settings.label("home_total_watches") << catalog.getAll().size() << "\n"
                  << settings.label("home_language_label") << languageDisplayName(settings.getLanguage())
                  << "  |  " << settings.label("home_currency_label") << currencyDisplayName(settings.getCurrency()) << "\n"
                  << settings.label("home_tip") << "\n";
    }

    // Shared by every browsing sub-menu: prints the given list of watches
    // and lets the user add one of them to the cart by id, specifying how
    // many units they want (project requirement 3).
    void browseAndMaybeAdd(const std::vector<Watch>& watches) {
        if (watches.empty()) {
            std::cout << settings.label("no_watches_found") << "\n";
            return;
        }
        for (const auto& w : watches) {
            w.display(settings);
        }

        std::cout << "\n" << settings.label("add_to_cart_prompt");
        int id;
        if (!(std::cin >> id)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return;
        }
        if (id == 0) {
            return;
        }

        const Watch* found = catalog.findById(id);
        if (found) {
            std::cout << settings.label("quantity_prompt");
            int qty = readQuantity();
            cart.addItem(*found, qty, settings);
        } else {
            std::cout << settings.label("no_watch_shown_here") << "\n";
        }
    }

    // "Brand" sub-menu: shows every watch in the store regardless of
    // category, so the user can shop purely by brand.
    void handleBrandMenu() {
        std::cout << "\n--- " << settings.label("menu_brand") << ": " << settings.label("brand_all_watches") << " ---\n";
        browseAndMaybeAdd(catalog.getAll());
    }

    // Shared implementation backing the Men / Women / Other sub-menus.
    void handleCategoryMenu(Category category, const std::string& title) {
        std::cout << "\n--- " << title << " ---\n";
        browseAndMaybeAdd(catalog.getByCategory(category));
    }

    // Search sub-menu: free-text, case-insensitive brand search.
    void handleSearch() {
        std::cout << "\n--- " << settings.label("menu_search") << " ---\n"
                  << settings.label("search_brand_prompt");
        std::string keyword;
        std::cin >> keyword;

        std::vector<Watch> results = catalog.searchByBrand(keyword);
        std::cout << settings.label("search_found_prefix") << results.size()
                  << settings.label("search_found_suffix") << "\n";
        browseAndMaybeAdd(results);
    }

    // Cart sub-menu: view current items and optionally update the
    // quantity of, or remove, one of them.
    void handleCart() {
        cart.display(settings);
        if (cart.isEmpty()) {
            return;
        }

        std::cout << "\n1. " << settings.label("cart_menu_update_qty")
                  << "\n2. " << settings.label("cart_menu_remove_item")
                  << "\n3. " << settings.label("admin_back_to_main") << "\n";
        int choice = readIntChoice(1, 3);
        if (choice == 1) {
            std::cout << settings.label("cart_update_id_prompt");
            int id;
            std::cin >> id;
            std::cout << settings.label("cart_new_qty_prompt");
            int qty;
            std::cin >> qty;
            if (cart.setQuantity(id, qty)) {
                std::cout << settings.label("cart_updated_msg") << "\n";
            } else {
                std::cout << settings.label("cart_item_not_found") << "\n";
            }
        } else if (choice == 2) {
            std::cout << settings.label("cart_remove_id_prompt");
            int id;
            std::cin >> id;
            if (cart.removeItem(id)) {
                std::cout << settings.label("cart_removed_msg") << "\n";
            } else {
                std::cout << settings.label("cart_item_not_found") << "\n";
            }
        }
    }

    // Prompts the customer for delivery details (recipient name, phone
    // number, and shipping address/city). Pre-fills each field with the
    // previously entered value (if any) so repeat checkouts are quick --
    // press Enter on a blank line to keep the existing value.
    ShippingAddress collectShippingAddress() {
        std::cout << "\n" << settings.label("shipping_title") << "\n";
        ShippingAddress addr = lastShippingAddress;

        primeForLineInput(); // discard the leftover newline from the prior cin >> read
        std::cout << settings.label("shipping_recipient")
                   << (addr.recipientName.empty() ? "" : ("[" + addr.recipientName + "] "));
        std::string line = readLine();
        if (!line.empty()) addr.recipientName = line;

        std::cout << settings.label("shipping_phone")
                   << (addr.phone.empty() ? "" : ("[" + addr.phone + "] "));
        line = readLine();
        if (!line.empty()) addr.phone = line;

        std::cout << settings.label("shipping_address")
                   << (addr.addressLine.empty() ? "" : ("[" + addr.addressLine + "] "));
        line = readLine();
        if (!line.empty()) addr.addressLine = line;

        lastShippingAddress = addr;
        return addr;
    }

    // Checkout sub-menu: confirms the cart, collects the shipping address,
    // delegates payment-method selection and processing to the Payment
    // class, prints a final order summary (cart + shipping + payment),
    // then clears the cart once payment succeeds.
    void handleCheckout() {
        if (cart.isEmpty()) {
            std::cout << settings.label("checkout_cart_empty") << "\n";
            return;
        }

        cart.display(settings);
        std::cout << "\n" << settings.label("checkout_proceed_payment") << "\n";
        if (readIntChoice(1, 2) == 2) {
            return;
        }

        ShippingAddress shipping = collectShippingAddress();

        PaymentMethod method = Payment::selectMethod(settings);
        bool success = Payment::process(method, cart.getTotal(), settings);

        if (success) {
            std::cout << "\n" << settings.label("order_summary_title") << "\n";
            cart.display(settings);
            std::cout << "\n" << settings.label("shipping_title") << "\n";
            shipping.display(settings);
            std::cout << "\n" << settings.label("checkout_payment_method") << Payment::methodToString(method) << "\n";
            std::cout << settings.label("checkout_amount_charged") << settings.formatPrice(cart.getTotal()) << "\n";

            // Deduct the purchased quantities from the master inventory and
            // persist the updated stock levels to inventory.txt so the
            // reduction survives across program runs.
            for (const auto& item : cart.getItems()) {
                catalog.deductStock(item.watch.getId(), item.quantity);
            }
            catalog.saveToFile();

            // Admin Stock-Refill Notification: now that the deduction is
            // persisted, warn the admin about any purchased item that has
            // dropped below the restock threshold. This is a backend/admin
            // concern, so it must never be shown to a Customer -- only
            // surface it when the account performing checkout is an Admin.
            if (currentRole == Role::Admin) {
                notifyLowStock(cart.getItems());
            }

            cart.clear();
        }
    }

    // Admin Stock-Refill Notification: given the line items just
    // purchased, looks up each product's post-deduction stock in the
    // catalog and prints a console alert for any that fell below
    // LOW_STOCK_THRESHOLD. Intended to be called right after a
    // checkout's inventory deduction has been persisted.
    void notifyLowStock(const std::vector<CartItem>& purchasedItems) {
        std::vector<const Watch*> lowStockItems;
        for (const auto& item : purchasedItems) {
            const Watch* current = catalog.findById(item.watch.getId());
            if (current && current->getStock() < LOW_STOCK_THRESHOLD) {
                lowStockItems.push_back(current);
            }
        }

        if (lowStockItems.empty()) {
            return;
        }

        std::cout << "\n" << settings.label("low_stock_alert_title") << "\n";
        std::cout << settings.label("low_stock_alert_intro") << "\n";
        for (const Watch* w : lowStockItems) {
            std::cout << "  - [" << w->getId() << "] " << w->getBrand() << " " << w->getName()
                      << settings.label("low_stock_alert_only") << w->getStock()
                      << settings.label("low_stock_alert_item_suffix") << "\n";
        }
        std::cout << "***************************************\n";
    }

    // "Settings" sub-menu (project requirement 2): lets the user change
    // the UI language. Currency is never chosen directly here -- picking a
    // language automatically re-maps it to that region's currency. Use the
    // separate "Currency" main-menu option to set a currency (e.g. MMK)
    // independently of language.
    void handleSettingsMenu() {
        std::cout << "\n" << settings.label("settings_title") << "\n"
                  << settings.label("settings_current_lang") << languageDisplayName(settings.getLanguage()) << "\n"
                  << settings.label("settings_current_currency") << currencyDisplayName(settings.getCurrency()) << "\n\n"
                  << settings.label("settings_choose") << "\n"
                  << "1. English (en)\n"
                  << "2. Chinese (ch)\n"
                  << "3. Japanese (jp)\n"
                  << "4. Italian (it)\n"
                  << "5. French (fr)\n"
                  << "6. " << settings.label("settings_back") << "\n";
        int choice = readIntChoice(1, 6);
        if (choice == 6) {
            return;
        }

        Language newLang = Language::English;
        switch (choice) {
            case 1: newLang = Language::English;  break;
            case 2: newLang = Language::Chinese;  break;
            case 3: newLang = Language::Japanese; break;
            case 4: newLang = Language::Italian;  break;
            case 5: newLang = Language::French;   break;
        }

        settings.setLanguage(newLang); // currency auto-updates with the language
        std::cout << settings.label("settings_updated") << languageDisplayName(settings.getLanguage())
                  << settings.label("settings_currency_auto") << currencyDisplayName(settings.getCurrency()) << "\n";
    }

    // "Currency" sub-menu (Myanmar Kyat support): lets the user pick any
    // supported display currency directly, independent of UI language.
    void handleCurrencyMenu() {
        std::cout << "\n" << settings.label("currency_title") << "\n"
                  << settings.label("settings_current_currency") << currencyDisplayName(settings.getCurrency()) << "\n\n"
                  << settings.label("currency_choose") << "\n"
                  << "1. " << currencyDisplayName(Currency::USD) << "\n"
                  << "2. " << currencyDisplayName(Currency::CNY) << "\n"
                  << "3. " << currencyDisplayName(Currency::JPY) << "\n"
                  << "4. " << currencyDisplayName(Currency::EUR) << "\n"
                  << "5. " << currencyDisplayName(Currency::MMK) << " (Myanmar Kyat)\n"
                  << "6. " << settings.label("settings_back") << "\n";
        int choice = readIntChoice(1, 6);
        if (choice == 6) {
            return;
        }

        Currency newCurrency = Currency::USD;
        switch (choice) {
            case 1: newCurrency = Currency::USD; break;
            case 2: newCurrency = Currency::CNY; break;
            case 3: newCurrency = Currency::JPY; break;
            case 4: newCurrency = Currency::EUR; break;
            case 5: newCurrency = Currency::MMK; break;
        }

        settings.setCurrency(newCurrency);
        std::cout << settings.label("currency_set_to") << currencyDisplayName(settings.getCurrency()) << ".\n";
    }

    // =========================== ADMIN PANEL ================================
    // Everything below is reachable only from run()'s admin-only branch, so
    // Regular Customers have no menu path into any of it (RBAC requirement).

    // Top-level Admin Panel: routes to user management or inventory
    // management, or back to the main menu.
    void handleAdminPanel() {
        bool inPanel = true;
        while (inPanel) {
            std::cout << "\n" << settings.label("admin_panel_title") << "\n"
                      << "1. " << settings.label("admin_option_users") << "\n"
                      << "2. " << settings.label("admin_option_inventory") << "\n"
                      << "3. " << settings.label("admin_back_to_main") << "\n";
            switch (readIntChoice(1, 3)) {
                case 1: handleUserManagement(); break;
                case 2: handleInventoryManagement(); break;
                case 3: inPanel = false; break;
            }
        }
    }

    // Admin Management Feature: view, add, and remove whitelisted accounts.
    // Every add/remove is immediately persisted to users.txt by User::addUser/removeUser.
    void handleUserManagement() {
        bool inMenu = true;
        while (inMenu) {
            std::cout << "\n--- " << settings.label("admin_option_users") << " (" << Storage::USERS_FILE << ") ---\n";
            for (const auto& u : User::listUsers()) {
                std::cout << "  " << u.getUsername() << "  (" << roleToString(u.getRole()) << ")\n";
            }
            std::cout << "\n1. " << settings.label("admin_menu_add_account")
                      << "\n2. " << settings.label("admin_menu_remove_account")
                      << "\n3. " << settings.label("settings_back") << "\n";
            int choice = readIntChoice(1, 3);

            if (choice == 1) {
                std::string username, password;
                std::cout << settings.label("admin_new_username");
                std::cin >> username;
                std::cout << settings.label("admin_new_password");
                std::cin >> password;
                std::cout << settings.label("admin_role_prompt");
                int roleChoice = readIntChoice(1, 2);
                Role role = (roleChoice == 1) ? Role::Admin : Role::Customer;

                if (User::addUser(username, password, role)) {
                    std::cout << settings.label("admin_account_added_prefix") << username
                              << settings.label("admin_account_added_as") << roleToString(role)
                              << settings.label("admin_account_added_saved") << Storage::USERS_FILE << ".\n";
                } else {
                    std::cout << settings.label("admin_username_exists") << "\n";
                }
            } else if (choice == 2) {
                std::cout << settings.label("admin_remove_username_prompt");
                std::string username;
                std::cin >> username;
                std::string reason;
                if (User::removeUser(username, reason)) {
                    std::cout << settings.label("admin_account_added_prefix") << username
                              << settings.label("admin_account_removed_suffix") << Storage::USERS_FILE
                              << settings.label("admin_account_removed_updated") << "\n";
                } else {
                    std::cout << settings.label("admin_remove_failed") << settings.label(reason) << "\n";
                }
            } else {
                inMenu = false;
            }
        }
    }

    // Admin Management Feature: add, update, or delete watches in the
    // catalog. Every add/delete is immediately persisted to inventory.txt;
    // in-place field edits are saved once the admin finishes editing a
    // product (see adminUpdateProduct()).
    void handleInventoryManagement() {
        bool inMenu = true;
        while (inMenu) {
            std::cout << "\n--- " << settings.label("admin_option_inventory") << " (" << Storage::INVENTORY_FILE << ") ---\n";
            for (const auto& w : catalog.getAll()) {
                w.display(settings);
            }
            std::cout << "\n1. " << settings.label("inv_menu_add_product")
                      << "\n2. " << settings.label("inv_menu_update_product")
                      << "\n3. " << settings.label("inv_menu_delete_product")
                      << "\n4. " << settings.label("settings_back") << "\n";
            int choice = readIntChoice(1, 4);

            if (choice == 1) {
                adminAddProduct();
            } else if (choice == 2) {
                adminUpdateProduct();
            } else if (choice == 3) {
                adminDeleteProduct();
            } else {
                inMenu = false;
            }
        }
    }

    // Prompts for brand/name/price/category/stock and adds the new watch.
    void adminAddProduct() {
        std::string brand, name;
        double price;
        int stock;
        std::cout << settings.label("inv_prompt_brand");
        std::cin >> brand;
        std::cout << settings.label("inv_prompt_model");
        std::cin >> name;
        std::cout << settings.label("inv_prompt_price");
        while (!(std::cin >> price) || price < 0) {
            std::cout << settings.label("inv_invalid_price");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        Category category = promptCategory();
        std::cout << settings.label("inv_prompt_stock");
        while (!(std::cin >> stock) || stock < 0) {
            std::cout << settings.label("inv_invalid_stock");
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        int newId = catalog.addWatch(brand, name, price, category, stock);
        std::cout << settings.label("inv_added_prefix") << newId
                  << settings.label("admin_account_added_saved") << Storage::INVENTORY_FILE << ".\n";
    }

    // Looks up a product by id, then lets the admin repeatedly edit
    // individual fields until they choose "Done". Persists once at the end.
    void adminUpdateProduct() {
        std::cout << settings.label("inv_update_id_prompt");
        int id;
        std::cin >> id;
        Watch* found = catalog.findByIdMutable(id);
        if (!found) {
            std::cout << settings.label("inv_no_product") << "\n";
            return;
        }

        bool editing = true;
        bool changed = false;
        while (editing) {
            std::cout << "\n" << settings.label("inv_currently");
            found->display(settings);
            std::cout << "1. " << settings.label("inv_edit_brand")
                      << "\n2. " << settings.label("inv_edit_name")
                      << "\n3. " << settings.label("inv_edit_price")
                      << "\n4. " << settings.label("inv_edit_category")
                      << "\n5. " << settings.label("inv_edit_stock")
                      << "\n6. " << settings.label("inv_edit_done") << "\n";
            switch (readIntChoice(1, 6)) {
                case 1: {
                    std::string newBrand;
                    std::cout << settings.label("inv_new_brand");
                    std::cin >> newBrand;
                    found->setBrand(newBrand);
                    changed = true;
                    break;
                }
                case 2: {
                    std::string newName;
                    std::cout << settings.label("inv_new_model");
                    std::cin >> newName;
                    found->setName(newName);
                    changed = true;
                    break;
                }
                case 3: {
                    double newPrice;
                    std::cout << settings.label("inv_new_price");
                    while (!(std::cin >> newPrice) || newPrice < 0) {
                        std::cout << settings.label("inv_invalid_price");
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    }
                    found->setPrice(newPrice);
                    changed = true;
                    break;
                }
                case 4:
                    found->setCategory(promptCategory());
                    changed = true;
                    break;
                case 5: {
                    int newStock;
                    std::cout << settings.label("inv_new_stock");
                    while (!(std::cin >> newStock) || newStock < 0) {
                        std::cout << settings.label("inv_invalid_stock");
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    }
                    found->setStock(newStock);
                    changed = true;
                    break;
                }
                case 6:
                    editing = false;
                    break;
            }
        }
        if (changed) {
            catalog.saveToFile();
            std::cout << settings.label("inv_updated_saved") << Storage::INVENTORY_FILE << ".\n";
        } else {
            std::cout << settings.label("inv_updated") << "\n";
        }
    }

    void adminDeleteProduct() {
        std::cout << settings.label("inv_delete_id_prompt");
        int id;
        std::cin >> id;
        if (catalog.deleteWatch(id)) {
            std::cout << settings.label("inv_deleted_saved") << Storage::INVENTORY_FILE
                      << settings.label("admin_account_removed_updated") << "\n";
        } else {
            std::cout << settings.label("inv_no_product") << "\n";
        }
    }

    // Small shared helper for the Add/Update product flows.
    Category promptCategory() {
        std::cout << settings.label("inv_category_prompt");
        switch (readIntChoice(1, 3)) {
            case 1: return Category::Men;
            case 2: return Category::Women;
            default: return Category::Other;
        }
    }
};

// ================================ 8. MAIN ===================================

// Entry point. All application logic is owned by MenuManager, which
// runs login -> main menu -> sub-menus -> checkout -> exit.
int main() {
    MenuManager app;
    app.run();
    return 0;
}