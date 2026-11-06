// Qt Widgets front end for TEAM CHRONOS.
// The original backend is included unchanged. Its console main() is renamed
// so this file can provide the Qt application's real entry point.
#define main team_chronos_console_main
#include "TEAM_CHRONOS.cpp"
#undef main

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QAbstractItemView>
#include <QSplitter>
#include <QStatusBar>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QVariant>
#include <QMap>
#include <QStringList>
#include <QSignalBlocker>
#include <QModelIndex>
#include <QItemSelectionModel>

#include <memory>

static QString qs(const std::string& value) {
    return QString::fromUtf8(value.c_str());
}

static std::string ss(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    return std::string(bytes.constData(),
                       static_cast<std::size_t>(bytes.size()));
}

class LoginDialog final : public QDialog {
public:
    explicit LoginDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("TEAM CHRONOS — Sign in");
        setMinimumWidth(360);

        auto* layout = new QVBoxLayout(this);
        auto* heading = new QLabel("TEAM CHRONOS");

        heading->setAlignment(Qt::AlignCenter);
        heading->setStyleSheet(
            "font-size: 24px;"
            "font-weight: 700;"
            "padding: 12px;"
        );

        layout->addWidget(heading);

        auto* form = new QFormLayout;

        username_ = new QLineEdit;
        password_ = new QLineEdit;

        password_->setEchoMode(QLineEdit::Password);
        username_->setPlaceholderText("Username");
        password_->setPlaceholderText("Password");

        form->addRow("Username", username_);
        form->addRow("Password", password_);

        layout->addLayout(form);

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
        );

        connect(buttons, &QDialogButtonBox::accepted,
                this, &QDialog::accept);

        connect(buttons, &QDialogButtonBox::rejected,
                this, &QDialog::reject);

        layout->addWidget(buttons);

        password_->setFocus();
    }

    QString username() const {
        return username_->text().trimmed();
    }

    QString password() const {
        return password_->text();
    }

private:
    QLineEdit* username_{};
    QLineEdit* password_{};
};


class StoreWindow final : public QMainWindow {
public:
    StoreWindow(std::string username, Role role)
        : username_(std::move(username)),
          role_(role),
          catalog_(),
          cart_(),
          settings_() {

        setWindowTitle("TEAM CHRONOS — Watch Store");
        resize(1240, 760);

        buildUi();
        refreshAll();

        statusBar()->showMessage(
            tr("signed_in") + ": " + qs(username_)
        );
    }

private:
    std::string username_;
    Role role_;

    WatchCatalog catalog_;
    Cart cart_;
    Settings settings_;

    QLineEdit* searchEdit_{};
    QComboBox* categoryFilter_{};
    QComboBox* currencyCombo_{};
    QComboBox* languageCombo_{};

    QLabel* titleLabel_{};
    QLabel* currencyLabel_{};
    QLabel* languageLabel_{};
    QLabel* cartHeading_{};

    QPushButton* addButton_{};
    QPushButton* refreshButton_{};
    QPushButton* updateButton_{};
    QPushButton* removeButton_{};
    QPushButton* clearButton_{};
    QPushButton* checkoutButton_{};

    QPushButton* addProductButton_{};
    QPushButton* updateProductButton_{};
    QPushButton* deleteProductButton_{};
    QPushButton* usersButton_{};

    QTableWidget* catalogTable_{};
    QTableWidget* cartTable_{};

    QLabel* cartTotalLabel_{};
    QLabel* userLabel_{};


    // ---------------------------------------------------------
    // BUILD MAIN WINDOW
    // ---------------------------------------------------------

    void buildUi() {
        auto* central = new QWidget;
        auto* root = new QVBoxLayout(central);

        // TOP BAR
        auto* top = new QHBoxLayout;

        titleLabel_ = new QLabel("TEAM CHRONOS");

        titleLabel_->setStyleSheet(
            "font-size: 25px;"
            "font-weight: 800;"
            "letter-spacing: 2px;"
        );

        userLabel_ = new QLabel;

        currencyCombo_ = new QComboBox;
        currencyCombo_->addItems({
            "USD", "CNY", "JPY", "EUR", "MMK"
        });

        languageCombo_ = new QComboBox;
        languageCombo_->addItems({
            "English",
            "Chinese",
            "Japanese",
            "Italian",
            "French"
        });

        currencyLabel_ = new QLabel;
        languageLabel_ = new QLabel;

        top->addWidget(titleLabel_);
        top->addStretch();
        top->addWidget(userLabel_);
        top->addSpacing(12);
        top->addWidget(languageLabel_);
        top->addWidget(languageCombo_);
        top->addWidget(currencyLabel_);
        top->addWidget(currencyCombo_);

        root->addLayout(top);


        // MAIN SPLITTER
        auto* splitter = new QSplitter(Qt::Horizontal);

        // WATCH CATALOG
        auto* catalogPane = new QWidget;
        auto* catalogLayout = new QVBoxLayout(catalogPane);

        auto* filterRow = new QHBoxLayout;

        searchEdit_ = new QLineEdit;
        searchEdit_->setPlaceholderText(
            "Search brand or model..."
        );

        categoryFilter_ = new QComboBox;
        categoryFilter_->addItems({
            "All categories", "Men", "Women", "Other"
        });

        filterRow->addWidget(searchEdit_, 1);
        filterRow->addWidget(categoryFilter_);

        catalogLayout->addLayout(filterRow);

        catalogTable_ = new QTableWidget(0, 6);

        catalogTable_->setHorizontalHeaderLabels({
            "ID", "Brand", "Model", "Category", "Price", "Stock"
        });

        configureTable(catalogTable_);
        catalogLayout->addWidget(catalogTable_);

        auto* catalogButtons = new QHBoxLayout;

        addButton_ = new QPushButton;
        refreshButton_ = new QPushButton;

        catalogButtons->addWidget(addButton_);
        catalogButtons->addWidget(refreshButton_);

        catalogLayout->addLayout(catalogButtons);

        splitter->addWidget(catalogPane);


        // SHOPPING CART
        auto* cartPane = new QWidget;
        auto* cartLayout = new QVBoxLayout(cartPane);

        cartHeading_ = new QLabel;
        cartHeading_->setStyleSheet(
            "font-size: 18px;"
            "font-weight: 700;"
        );

        cartLayout->addWidget(cartHeading_);

        cartTable_ = new QTableWidget(0, 5);

        cartTable_->setHorizontalHeaderLabels({
            "ID", "Watch", "Qty", "Unit price", "Subtotal"
        });

        configureTable(cartTable_);
        cartLayout->addWidget(cartTable_);

        cartTotalLabel_ = new QLabel("Total: $0.00");
        cartTotalLabel_->setAlignment(Qt::AlignRight);

        cartTotalLabel_->setStyleSheet(
            "font-size: 18px;"
            "font-weight: 700;"
            "padding: 8px;"
        );

        cartLayout->addWidget(cartTotalLabel_);

        auto* cartButtons = new QHBoxLayout;

        updateButton_ = new QPushButton;
        removeButton_ = new QPushButton;
        clearButton_ = new QPushButton;

        cartButtons->addWidget(updateButton_);
        cartButtons->addWidget(removeButton_);
        cartButtons->addWidget(clearButton_);

        cartLayout->addLayout(cartButtons);

        checkoutButton_ = new QPushButton;
        checkoutButton_->setMinimumHeight(42);
        checkoutButton_->setStyleSheet("font-weight: 700;");

        cartLayout->addWidget(checkoutButton_);

        splitter->addWidget(cartPane);

        splitter->setStretchFactor(0, 3);
        splitter->setStretchFactor(1, 2);

        root->addWidget(splitter, 1);


        // ADMIN BUTTONS
        auto* bottom = new QHBoxLayout;

        if (role_ == Role::Admin) {
            addProductButton_ = new QPushButton;
            updateProductButton_ = new QPushButton;
            deleteProductButton_ = new QPushButton;
            usersButton_ = new QPushButton;

            bottom->addWidget(addProductButton_);
            bottom->addWidget(updateProductButton_);
            bottom->addWidget(deleteProductButton_);
            bottom->addWidget(usersButton_);

            connect(
                addProductButton_,
                &QPushButton::clicked,
                this,
                [this] { addProduct(); }
            );

            connect(
                updateProductButton_,
                &QPushButton::clicked,
                this,
                [this] { updateProduct(); }
            );

            connect(
                deleteProductButton_,
                &QPushButton::clicked,
                this,
                [this] { deleteProduct(); }
            );

            connect(
                usersButton_,
                &QPushButton::clicked,
                this,
                [this] { manageUsers(); }
            );
        }

        bottom->addStretch();
        root->addLayout(bottom);

        setCentralWidget(central);

        retranslateUi();


        // SEARCH AND FILTER
        connect(
            searchEdit_,
            &QLineEdit::textChanged,
            this,
            [this] { refreshCatalog(); }
        );

        connect(
            categoryFilter_,
            &QComboBox::currentTextChanged,
            this,
            [this] { refreshCatalog(); }
        );


        // CURRENCY
        connect(
            currencyCombo_,
            &QComboBox::currentTextChanged,
            this,
            [this](const QString& value) {
                if (value == "USD")
                    settings_.setCurrency(Currency::USD);
                else if (value == "CNY")
                    settings_.setCurrency(Currency::CNY);
                else if (value == "JPY")
                    settings_.setCurrency(Currency::JPY);
                else if (value == "EUR")
                    settings_.setCurrency(Currency::EUR);
                else if (value == "MMK")
                    settings_.setCurrency(Currency::MMK);

                refreshAll();
            }
        );


        // LANGUAGE
        connect(
            languageCombo_,
            &QComboBox::currentIndexChanged,
            this,
            [this](int index) {
                switch (index) {
                    case 0:
                        settings_.setLanguage(Language::English);
                        break;
                    case 1:
                        settings_.setLanguage(Language::Chinese);
                        break;
                    case 2:
                        settings_.setLanguage(Language::Japanese);
                        break;
                    case 3:
                        settings_.setLanguage(Language::Italian);
                        break;
                    case 4:
                        settings_.setLanguage(Language::French);
                        break;
                    default:
                        settings_.setLanguage(Language::English);
                        break;
                }

                const QSignalBlocker blocker(currencyCombo_);

                currencyCombo_->setCurrentText(
                    qs(currencyCode(settings_.getCurrency()))
                );

                retranslateUi();
                refreshAll();
            }
        );


        // CUSTOMER BUTTONS
        connect(
            addButton_,
            &QPushButton::clicked,
            this,
            [this] { addSelectedToCart(); }
        );

        connect(
            refreshButton_,
            &QPushButton::clicked,
            this,
            [this] { refreshAll(); }
        );

        connect(
            updateButton_,
            &QPushButton::clicked,
            this,
            [this] { updateCartQuantity(); }
        );

        connect(
            removeButton_,
            &QPushButton::clicked,
            this,
            [this] { removeCartItem(); }
        );

        connect(
            clearButton_,
            &QPushButton::clicked,
            this,
            [this] {
                cart_.clear();
                refreshCart();
            }
        );

        connect(
            checkoutButton_,
            &QPushButton::clicked,
            this,
            [this] { checkout(); }
        );

        catalogTable_->setSelectionBehavior(
            QAbstractItemView::SelectRows
        );

        cartTable_->setSelectionBehavior(
            QAbstractItemView::SelectRows
        );
    }


    // ---------------------------------------------------------
    // TRANSLATIONS
    // ---------------------------------------------------------

    QString tr(const QString& key) const {
        const std::string backend = settings_.label(ss(key));

        if (backend != ss(key))
            return qs(backend);

        const int lang =
            static_cast<int>(settings_.getLanguage());

        static const QMap<QString, QStringList> translations = {
            {"watch_store", {"Watch Store", "手表商店", "時計店", "Negozio di orologi", "Boutique de montres"}},
            {"units", {"units", "件", "点", "unità", "unités"}},
            {"brand", {"Brand", "品牌", "ブランド", "Marca", "Marque"}},
            {"model", {"Model", "型号", "モデル", "Modello", "Modèle"}},
            {"price_usd", {"Price (USD)", "价格（美元）", "価格（USD）", "Prezzo (USD)", "Prix (USD)"}},
            {"category", {"Category", "类别", "カテゴリー", "Categoria", "Catégorie"}},
            {"stock", {"Stock", "库存", "在庫", "Scorte", "Stock"}},
            {"username", {"Username", "用户名", "ユーザー名", "Nome utente", "Nom d'utilisateur"}},
            {"password", {"Password", "密码", "パスワード", "Password", "Mot de passe"}},
            {"role", {"Role", "角色", "役割", "Ruolo", "Rôle"}},
            {"choose_account", {"Choose account:", "选择账户：", "アカウントを選択：", "Scegli account:", "Choisir un compte :"}},
            {"cannot_remove", {"Cannot remove account", "无法删除账户", "アカウントを削除できません", "Impossibile rimuovere l'account", "Impossible de supprimer le compte"}},
            {"new_product_saved", {"New product saved with ID", "新商品已保存，编号", "新しい商品を保存しました。ID", "Nuovo prodotto salvato con ID", "Nouveau produit enregistré avec l'ID"}},
            {"delete_id", {"Delete product ID", "删除商品编号", "商品IDを削除", "Eliminare il prodotto ID", "Supprimer le produit ID"}},
            {"payment_success_msg", {"Payment successful (simulated).", "付款成功（模拟）。", "支払い成功（シミュレーション）。", "Pagamento riuscito (simulato).", "Paiement réussi (simulation)."}},
            {"charged", {"Charged", "已支付", "請求額", "Addebitato", "Débité"}},
            {"method", {"Method", "方式", "方法", "Metodo", "Méthode"}},
            {"low_stock_alert", {"Admin low-stock alert:", "管理员低库存提醒：", "管理者向け低在庫アラート：", "Avviso scorte basse amministratore:", "Alerte de stock faible administrateur :"}},
            {"left", {"left", "剩余", "残り", "rimasti", "restants"}},
            {"ok", {"OK", "确定", "OK", "OK", "OK"}},
            {"save", {"Save", "保存", "保存", "Salva", "Enregistrer"}},
            {"cancel", {"Cancel", "取消", "キャンセル", "Annulla", "Annuler"}},
            {"currency", {"Currency", "货币", "通貨", "Valuta", "Devise"}},
            {"language", {"Language", "语言", "言語", "Lingua", "Langue"}},
            {"signed_in", {"Signed in", "已登录", "ログイン中", "Accesso effettuato", "Connecté"}},
            {"admin", {"Admin", "管理员", "管理者", "Amministratore", "Administrateur"}},
            {"customer", {"Customer", "顾客", "顧客", "Cliente", "Client"}},
            {"search", {"Search brand or model...", "搜索品牌或型号...", "ブランドまたはモデルを検索...", "Cerca marca o modello...", "Rechercher marque ou modèle..."}},
            {"all_categories", {"All categories", "所有类别", "すべてのカテゴリー", "Tutte le categorie", "Toutes les catégories"}},
            {"men", {"Men", "男士", "メンズ", "Uomo", "Homme"}},
            {"women", {"Women", "女士", "レディース", "Donna", "Femme"}},
            {"other", {"Other", "其他", "その他", "Altro", "Autre"}},
            {"catalog_headers", {"ID|Brand|Model|Category|Price|Stock", "编号|品牌|型号|类别|价格|库存", "ID|ブランド|モデル|カテゴリー|価格|在庫", "ID|Marca|Modello|Categoria|Prezzo|Scorte", "ID|Marque|Modèle|Catégorie|Prix|Stock"}},
            {"cart_headers", {"ID|Watch|Qty|Unit price|Subtotal", "编号|手表|数量|单价|小计", "ID|時計|数量|単価|小計", "ID|Orologio|Qtà|Prezzo unitario|Subtotale", "ID|Montre|Qté|Prix unitaire|Sous-total"}},
            {"add_cart", {"Add selected to cart", "将所选商品加入购物车", "選択した商品をカートに追加", "Aggiungi selezionato al carrello", "Ajouter la sélection au panier"}},
            {"refresh", {"Refresh", "刷新", "更新", "Aggiorna", "Actualiser"}},
            {"cart_title", {"SHOPPING CART", "购物车", "ショッピングカート", "CARRELLO", "PANIER"}},
            {"total", {"Total", "总计", "合計", "Totale", "Total"}},
            {"update_qty", {"Update quantity", "更新数量", "数量を更新", "Aggiorna quantità", "Modifier la quantité"}},
            {"remove_item", {"Remove item", "移除商品", "商品を削除", "Rimuovi articolo", "Retirer l'article"}},
            {"clear_cart", {"Clear cart", "清空购物车", "カートを空にする", "Svuota carrello", "Vider le panier"}},
            {"checkout", {"Proceed to checkout", "前往结账", "レジに進む", "Procedi al pagamento", "Passer à la caisse"}},
            {"add_product", {"Add product", "添加商品", "商品を追加", "Aggiungi prodotto", "Ajouter un produit"}},
            {"update_product", {"Update product", "更新商品", "商品を更新", "Aggiorna prodotto", "Modifier le produit"}},
            {"product_updated", {"Product updated", "商品已更新", "商品を更新しました", "Prodotto aggiornato", "Produit mis à jour"}},
            {"product_update_saved", {"Product details were saved to inventory.txt.", "商品信息已保存到 inventory.txt。", "商品情報を inventory.txt に保存しました。", "I dettagli del prodotto sono stati salvati in inventory.txt.", "Les informations du produit ont été enregistrées dans inventory.txt."}},
            {"delete_product", {"Delete selected product", "删除所选商品", "選択した商品を削除", "Elimina prodotto selezionato", "Supprimer le produit sélectionné"}},
            {"manage_users", {"Manage users", "管理用户", "ユーザー管理", "Gestisci utenti", "Gérer les utilisateurs"}},
            {"select_watch_title", {"Select a watch", "选择手表", "時計を選択", "Seleziona un orologio", "Sélectionner une montre"}},
            {"select_watch_msg", {"Choose a watch from the catalog first.", "请先从目录中选择一款手表。", "先にカタログから時計を選択してください。", "Seleziona prima un orologio dal catalogo.", "Choisissez d'abord une montre dans le catalogue."}},
            {"quantity", {"Quantity:", "数量：", "数量：", "Quantità:", "Quantité :"}},
            {"insufficient_stock", {"Insufficient stock", "库存不足", "在庫不足", "Scorte insufficienti", "Stock insuffisant"}},
            {"requested_exceeds", {"Requested quantity exceeds available stock.", "请求数量超过可用库存。", "要求数量が在庫数を超えています。", "La quantità richiesta supera le scorte disponibili.", "La quantité demandée dépasse le stock disponible."}},
            {"update_title", {"Update quantity", "更新数量", "数量を更新", "Aggiorna quantità", "Modifier la quantité"}},
            {"new_quantity", {"New quantity (0 removes item):", "新数量（输入0可移除商品）：", "新しい数量（0で商品を削除）：", "Nuova quantità (0 rimuove l'articolo):", "Nouvelle quantité (0 supprime l'article) :"}},
            {"checkout_title", {"Checkout details", "结账信息", "チェックアウト情報", "Dettagli pagamento", "Détails du paiement"}},
            {"recipient", {"Recipient", "收件人", "受取人", "Destinatario", "Destinataire"}},
            {"phone", {"Phone", "电话", "電話", "Telefono", "Téléphone"}},
            {"address", {"Address", "地址", "住所", "Indirizzo", "Adresse"}},
            {"payment_method", {"Payment method", "付款方式", "支払い方法", "Metodo di pagamento", "Mode de paiement"}},
            {"missing_details", {"Missing details", "信息缺失", "入力不足", "Dati mancanti", "Informations manquantes"}},
            {"complete_shipping", {"Complete all shipping fields.", "请填写所有配送信息。", "配送情報をすべて入力してください。", "Completa tutti i campi di spedizione.", "Remplissez tous les champs de livraison."}},
            {"confirm_order", {"Confirm order", "确认订单", "注文を確認", "Conferma ordine", "Confirmer la commande"}},
            {"proceed_payment", {"Proceed with simulated payment?", "是否继续模拟付款？", "シミュレーション決済を続行しますか？", "Procedere con il pagamento simulato?", "Continuer avec le paiement simulé ?"}},
            {"payment_failed", {"Payment failed", "付款失败", "支払い失敗", "Pagamento non riuscito", "Échec du paiement"}},
            {"payment_not_completed", {"Payment was not completed.", "付款未完成。", "支払いは完了しませんでした。", "Il pagamento non è stato completato.", "Le paiement n'a pas été effectué."}},
            {"order_complete", {"Order complete", "订单完成", "注文完了", "Ordine completato", "Commande terminée"}},
            {"payment_success", {"Payment successful (simulated).", "付款成功（模拟）。", "支払い成功（シミュレーション）。", "Pagamento riuscito (simulato).", "Paiement réussi (simulation)."}},
            {"missing_info", {"Missing information", "缺少信息", "情報不足", "Informazioni mancanti", "Informations manquantes"}},
            {"brand_model_required", {"Brand and model are required.", "品牌和型号为必填项。", "ブランドとモデルは必須です。", "Marca e modello sono obbligatori.", "La marque et le modèle sont obligatoires."}},
            {"product_added", {"Product added", "商品已添加", "商品を追加しました", "Prodotto aggiunto", "Produit ajouté"}},
            {"select_product", {"Select a product", "选择商品", "商品を選択", "Seleziona un prodotto", "Sélectionner un produit"}},
            {"select_row", {"Select a catalog row first.", "请先选择目录中的一行。", "先にカタログの行を選択してください。", "Seleziona prima una riga del catalogo.", "Sélectionnez d'abord une ligne du catalogue."}},
            {"delete_product_title", {"Delete product", "删除商品", "商品を削除", "Elimina prodotto", "Supprimer le produit"}},
            {"user_management", {"User management", "用户管理", "ユーザー管理", "Gestione utenti", "Gestion des utilisateurs"}},
            {"action", {"Action:", "操作：", "操作：", "Azione:", "Action :"}},
            {"add_account", {"Add account", "添加账户", "アカウントを追加", "Aggiungi account", "Ajouter un compte"}},
            {"remove_account", {"Remove account", "删除账户", "アカウントを削除", "Rimuovi account", "Supprimer un compte"}},
            {"close", {"Close", "关闭", "閉じる", "Chiudi", "Fermer"}},
            {"user_exists", {"User exists", "用户已存在", "ユーザーは既に存在します", "Utente esistente", "Utilisateur existant"}},
            {"username_registered", {"That username is already registered.", "该用户名已注册。", "そのユーザー名は既に登録されています。", "Questo nome utente è già registrato.", "Ce nom d'utilisateur est déjà enregistré."}},
            {"user_added", {"User added", "用户已添加", "ユーザーを追加しました", "Utente aggiunto", "Utilisateur ajouté"}},
            {"account_saved", {"Account saved to users.txt.", "账户已保存到 users.txt。", "アカウントを users.txt に保存しました。", "Account salvato in users.txt.", "Compte enregistré dans users.txt."}},
            {"account_removed", {"Account removed", "账户已删除", "アカウントを削除しました", "Account rimosso", "Compte supprimé"}},
            {"account_removed_msg", {"Account removed from users.txt.", "账户已从 users.txt 删除。", "users.txt からアカウントを削除しました。", "Account rimosso da users.txt.", "Compte supprimé de users.txt."}}
        };

        const auto it = translations.constFind(key);

        if (it != translations.cend() &&
            lang >= 0 &&
            lang < it->size()) {
            return it->at(lang);
        }

        return key;
    }


    // ---------------------------------------------------------
    // REFRESH UI TEXT
    // ---------------------------------------------------------

    void retranslateUi() {
        setWindowTitle(
            QStringLiteral("TEAM CHRONOS — ") +
            tr("watch_store")
        );

        userLabel_->setText(
            tr("signed_in") + ": " +
            qs(username_) + "  •  " +
            (role_ == Role::Admin
                ? tr("admin")
                : tr("customer"))
        );

        languageLabel_->setText(tr("language"));
        currencyLabel_->setText(tr("currency"));

        searchEdit_->setPlaceholderText(tr("search"));

        categoryFilter_->blockSignals(true);

        const int categoryIndex =
            categoryFilter_->currentIndex();

        categoryFilter_->clear();

        categoryFilter_->addItems({
            tr("all_categories"),
            tr("men"),
            tr("women"),
            tr("other")
        });

        categoryFilter_->setCurrentIndex(
            qMax(0, categoryIndex)
        );

        categoryFilter_->blockSignals(false);

        catalogTable_->setHorizontalHeaderLabels(
            tr("catalog_headers").split('|')
        );

        cartTable_->setHorizontalHeaderLabels(
            tr("cart_headers").split('|')
        );

        addButton_->setText(tr("add_cart"));
        refreshButton_->setText(tr("refresh"));

        cartHeading_->setText(tr("cart_title"));

        updateButton_->setText(tr("update_qty"));
        removeButton_->setText(tr("remove_item"));
        clearButton_->setText(tr("clear_cart"));
        checkoutButton_->setText(tr("checkout"));

        if (addProductButton_)
            addProductButton_->setText(tr("add_product"));

        if (updateProductButton_)
            updateProductButton_->setText(tr("update_product"));

        if (deleteProductButton_)
            deleteProductButton_->setText(tr("delete_product"));

        if (usersButton_)
            usersButton_->setText(tr("manage_users"));

        refreshCart();
    }


    // ---------------------------------------------------------
    // TABLE CONFIGURATION
    // ---------------------------------------------------------

    static void configureTable(QTableWidget* table) {
        table->setEditTriggers(
            QAbstractItemView::NoEditTriggers
        );

        table->setSelectionBehavior(
            QAbstractItemView::SelectRows
        );

        table->setSelectionMode(
            QAbstractItemView::SingleSelection
        );

        table->verticalHeader()->setVisible(false);

        table->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch
        );

        table->setAlternatingRowColors(true);
    }


    // ---------------------------------------------------------
    // PRICE FORMATTING
    // ---------------------------------------------------------

    QString price(double usd) const {
        return qs(settings_.formatPrice(usd));
    }


    // ---------------------------------------------------------
    // REFRESH DATA
    // ---------------------------------------------------------

    void refreshAll() {
        refreshCatalog();
        refreshCart();
    }


    void refreshCatalog() {
        catalogTable_->setRowCount(0);

        const QString query =
            searchEdit_->text().trimmed().toLower();

        const int categoryIndex =
            categoryFilter_->currentIndex();

        for (const Watch& watch : catalog_.getAll()) {
            const QString brand = qs(watch.getBrand());
            const QString model = qs(watch.getName());

            const QString cat = qs(
                settings_.label(
                    categoryLocalizationKey(watch.getCategory())
                )
            );

            if (!query.isEmpty() &&
                !brand.toLower().contains(query) &&
                !model.toLower().contains(query)) {
                continue;
            }

            if (categoryIndex > 0 &&
                categoryIndex !=
                    static_cast<int>(watch.getCategory()) + 1) {
                continue;
            }

            const int row = catalogTable_->rowCount();

            catalogTable_->insertRow(row);

            setCell(
                catalogTable_, row, 0,
                QString::number(watch.getId())
            );

            setCell(catalogTable_, row, 1, brand);
            setCell(catalogTable_, row, 2, model);
            setCell(catalogTable_, row, 3, cat);

            setCell(
                catalogTable_, row, 4,
                price(watch.getPrice())
            );

            setCell(
                catalogTable_, row, 5,
                QString::number(watch.getStock())
            );
        }
    }


    void refreshCart() {
        cartTable_->setRowCount(0);

        for (const CartItem& item : cart_.getItems()) {
            const int row = cartTable_->rowCount();

            cartTable_->insertRow(row);

            setCell(
                cartTable_, row, 0,
                QString::number(item.watch.getId())
            );

            setCell(
                cartTable_, row, 1,
                qs(item.watch.getBrand() + " " + item.watch.getName())
            );

            setCell(
                cartTable_, row, 2,
                QString::number(item.quantity)
            );

            setCell(
                cartTable_, row, 3,
                price(item.watch.getPrice())
            );

            setCell(
                cartTable_, row, 4,
                price(item.watch.getPrice() * item.quantity)
            );
        }

        cartTotalLabel_->setText(
            tr("total") + ": " +
            price(cart_.getTotal()) +
            "  (" +
            QString::number(cart_.getTotalUnitCount()) +
            " " +
            tr("units") +
            ")"
        );
    }


    static void setCell(
        QTableWidget* table,
        int row,
        int column,
        const QString& value
    ) {
        auto* item = new QTableWidgetItem(value);

        if (column == 0) {
            item->setData(Qt::UserRole, value.toInt());
        }

        table->setItem(row, column, item);
    }


    int selectedId(QTableWidget* table) const {
        const int row = table->currentRow();

        if (row < 0 || !table->item(row, 0))
            return -1;

        return table->item(row, 0)->text().toInt();
    }


    // ---------------------------------------------------------
    // ADD TO CART
    // ---------------------------------------------------------

    void addSelectedToCart() {
        const int id = selectedId(catalogTable_);

        const Watch* watch = catalog_.findById(id);

        if (!watch) {
            QMessageBox::information(
                this,
                tr("select_watch_title"),
                tr("select_watch_msg")
            );

            return;
        }

        bool ok = false;

        const int qty = QInputDialog::getInt(
            this,
            tr("add_cart"),
            tr("quantity"),
            1,
            1,
            qMax(1, watch->getStock()),
            1,
            &ok
        );

        if (!ok)
            return;

        int alreadyInCart = 0;

        for (const auto& item : cart_.getItems()) {
            if (item.watch.getId() == id)
                alreadyInCart = item.quantity;
        }

        if (alreadyInCart + qty > watch->getStock()) {
            QMessageBox::warning(
                this,
                tr("insufficient_stock"),
                tr("requested_exceeds")
            );

            return;
        }

        cart_.addItem(*watch, qty, settings_);

        refreshCart();
    }


    // ---------------------------------------------------------
    // UPDATE CART QUANTITY
    // ---------------------------------------------------------

    void updateCartQuantity() {
        const int id = selectedId(cartTable_);

        if (id < 0)
            return;

        int current = 1;

        for (const auto& item : cart_.getItems()) {
            if (item.watch.getId() == id)
                current = item.quantity;
        }

        const Watch* watch = catalog_.findById(id);

        const int maxQty =
            watch ? watch->getStock() : current;

        bool ok = false;

        const int qty = QInputDialog::getInt(
            this,
            tr("update_title"),
            tr("new_quantity"),
            current,
            0,
            qMax(current, maxQty),
            1,
            &ok
        );

        if (!ok)
            return;

        if (watch && qty > watch->getStock()) {
            QMessageBox::warning(
                this,
                tr("insufficient_stock"),
                tr("requested_exceeds")
            );

            return;
        }

        cart_.setQuantity(id, qty);

        refreshCart();
    }


    // ---------------------------------------------------------
    // REMOVE CART ITEM
    // ---------------------------------------------------------

    void removeCartItem() {
        const int id = selectedId(cartTable_);

        if (id >= 0) {
            cart_.removeItem(id);
            refreshCart();
        }
    }


    // ---------------------------------------------------------
    // CHECKOUT
    // ---------------------------------------------------------

    void checkout() {
        if (cart_.isEmpty()) {
            QMessageBox::information(
                this,
                tr("checkout"),
                tr("cart_empty")
            );

            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle(tr("checkout_title"));

        auto* form = new QFormLayout(&dialog);

        QLineEdit recipient, phone, address;

        recipient.setPlaceholderText(tr("recipient"));
        phone.setPlaceholderText(tr("phone"));
        address.setPlaceholderText(tr("address"));

        QComboBox payment;

        payment.addItems({
            "KPay (KBZPay)",
            "AYA Pay",
            "CB Pay",
            "Wave Pay"
        });

        form->addRow(tr("recipient"), &recipient);
        form->addRow(tr("phone"), &phone);
        form->addRow(tr("address"), &address);
        form->addRow(tr("payment_method"), &payment);

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel
        );

        buttons->button(
            QDialogButtonBox::Ok
        )->setText(tr("ok"));

        buttons->button(
            QDialogButtonBox::Cancel
        )->setText(tr("cancel"));

        form->addRow(buttons);

        connect(
            buttons,
            &QDialogButtonBox::accepted,
            &dialog,
            &QDialog::accept
        );

        connect(
            buttons,
            &QDialogButtonBox::rejected,
            &dialog,
            &QDialog::reject
        );

        if (dialog.exec() != QDialog::Accepted)
            return;

        if (recipient.text().trimmed().isEmpty() ||
            phone.text().trimmed().isEmpty() ||
            address.text().trimmed().isEmpty()) {

            QMessageBox::warning(
                this,
                tr("missing_details"),
                tr("complete_shipping")
            );

            return;
        }

        const double total = cart_.getTotal();

        const auto answer = QMessageBox::question(
            this,
            tr("confirm_order"),
            tr("total") + ": " +
            price(total) + "\n" +
            tr("proceed_payment"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (answer != QMessageBox::Yes)
            return;

        PaymentMethod method = PaymentMethod::KPay;

        switch (payment.currentIndex()) {
            case 1:
                method = PaymentMethod::AyaPay;
                break;
            case 2:
                method = PaymentMethod::CBPay;
                break;
            case 3:
                method = PaymentMethod::WavePay;
                break;
        }

        if (!Payment::process(method, total, settings_)) {
            QMessageBox::warning(
                this,
                tr("payment_failed"),
                tr("payment_not_completed")
            );

            return;
        }

        QString lowStock;

        for (const auto& item : cart_.getItems()) {
            catalog_.deductStock(
                item.watch.getId(),
                item.quantity
            );

            const Watch* updated =
                catalog_.findById(item.watch.getId());

            if (role_ == Role::Admin &&
                updated &&
                updated->getStock() < 10) {

                lowStock += QString("%1 %2 — %3 %4\n")
                    .arg(
                        qs(updated->getBrand()),
                        qs(updated->getName())
                    )
                    .arg(updated->getStock())
                    .arg(tr("left"));
            }
        }

        catalog_.saveToFile();

        cart_.clear();

        refreshAll();

        QString summary =
            tr("payment_success_msg") +
            "\n\n" +
            tr("recipient") + ": " + recipient.text() +
            "\n" +
            tr("phone") + ": " + phone.text() +
            "\n" +
            tr("address") + ": " + address.text() +
            "\n" +
            tr("method") + ": " +
            qs(Payment::methodToString(method)) +
            "\n" +
            tr("charged") + ": " +
            price(total);

        if (!lowStock.isEmpty()) {
            summary +=
                "\n\n" +
                tr("low_stock_alert") +
                "\n" +
                lowStock;
        }

        QMessageBox::information(
            this,
            tr("order_complete"),
            summary
        );
    }


    // ---------------------------------------------------------
    // ADMIN: ADD PRODUCT
    // ---------------------------------------------------------

    void addProduct() {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("add_product"));

        auto* form = new QFormLayout(&dialog);

        QLineEdit brand, model;

        QDoubleSpinBox priceBox;
        priceBox.setRange(0, 100000000);
        priceBox.setDecimals(2);

        QComboBox category;
        category.addItems({
            tr("men"),
            tr("women"),
            tr("other")
        });

        QSpinBox stock;
        stock.setRange(0, 1000000);

        form->addRow(tr("brand"), &brand);
        form->addRow(tr("model"), &model);
        form->addRow(tr("price_usd"), &priceBox);
        form->addRow(tr("category"), &category);
        form->addRow(tr("stock"), &stock);

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel
        );

        buttons->button(
            QDialogButtonBox::Ok
        )->setText(tr("ok"));

        buttons->button(
            QDialogButtonBox::Cancel
        )->setText(tr("cancel"));

        form->addRow(buttons);

        connect(
            buttons,
            &QDialogButtonBox::accepted,
            &dialog,
            &QDialog::accept
        );

        connect(
            buttons,
            &QDialogButtonBox::rejected,
            &dialog,
            &QDialog::reject
        );

        if (dialog.exec() != QDialog::Accepted)
            return;

        if (brand.text().trimmed().isEmpty() ||
            model.text().trimmed().isEmpty()) {

            QMessageBox::warning(
                this,
                tr("missing_info"),
                tr("brand_model_required")
            );

            return;
        }

        Category cat =
            category.currentIndex() == 0 ? Category::Men :
            category.currentIndex() == 1 ? Category::Women :
            Category::Other;

        const int id = catalog_.addWatch(
            ss(brand.text().trimmed()),
            ss(model.text().trimmed()),
            priceBox.value(),
            cat,
            stock.value()
        );

        refreshCatalog();

        QMessageBox::information(
            this,
            tr("product_added"),
            tr("new_product_saved") +
            " " +
            QString::number(id) +
            "."
        );
    }


    // ---------------------------------------------------------
    // ADMIN: UPDATE PRODUCT
    // ---------------------------------------------------------

    void updateProduct() {

        // 1. Check whether a catalog row is selected.
        const QModelIndexList selectedRows =
            catalogTable_->selectionModel()->selectedRows();

        if (selectedRows.isEmpty()) {
            QMessageBox::information(
                this,
                tr("select_product"),
                tr("select_row")
            );

            return;
        }

        const int row = selectedRows.first().row();

        if (!catalogTable_->item(row, 0)) {
            QMessageBox::information(
                this,
                tr("select_product"),
                tr("select_row")
            );

            return;
        }

        const int id =
            catalogTable_->item(row, 0)->text().toInt();

        // 2. Retrieve the actual mutable product from the catalog.
        Watch* watch = catalog_.findByIdMutable(id);

        if (!watch) {
            QMessageBox::warning(
                this,
                tr("select_product"),
                tr("select_row")
            );

            return;
        }

        // 3. Create the edit dialog.
        // All fields are pre-filled with the selected product's details.
        QDialog dialog(this);

        dialog.setWindowTitle(tr("update_product"));
        dialog.setMinimumWidth(400);

        auto* form = new QFormLayout(&dialog);

        QLineEdit brand;
        QLineEdit model;

        QDoubleSpinBox priceBox;
        QComboBox category;
        QSpinBox stock;

        // Brand
        brand.setText(qs(watch->getBrand()));

        // Model
        model.setText(qs(watch->getName()));

        // Price
        priceBox.setRange(0.0, 100000000.0);
        priceBox.setDecimals(2);
        priceBox.setValue(watch->getPrice());

        // Category
        category.addItems({
            tr("men"),
            tr("women"),
            tr("other")
        });

        category.setCurrentIndex(
            watch->getCategory() == Category::Men ? 0 :
            watch->getCategory() == Category::Women ? 1 : 2
        );

        // Stock
        // Zero is allowed so an out-of-stock watch can be edited.
        stock.setRange(0, 1000000);
        stock.setValue(watch->getStock());

        form->addRow(tr("brand"), &brand);
        form->addRow(tr("model"), &model);
        form->addRow(tr("price_usd"), &priceBox);
        form->addRow(tr("category"), &category);
        form->addRow(tr("stock"), &stock);

        // Save / Cancel buttons
        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save |
            QDialogButtonBox::Cancel
        );

        buttons->button(
            QDialogButtonBox::Save
        )->setText(tr("save"));

        buttons->button(
            QDialogButtonBox::Cancel
        )->setText(tr("cancel"));

        form->addRow(buttons);

        connect(
            buttons,
            &QDialogButtonBox::accepted,
            &dialog,
            &QDialog::accept
        );

        connect(
            buttons,
            &QDialogButtonBox::rejected,
            &dialog,
            &QDialog::reject
        );

        // 4. Wait for the user to save or cancel.
        if (dialog.exec() != QDialog::Accepted)
            return;

        const QString newBrand =
            brand.text().trimmed();

        const QString newModel =
            model.text().trimmed();

        // Validate required fields.
        if (newBrand.isEmpty() || newModel.isEmpty()) {
            QMessageBox::warning(
                this,
                tr("missing_info"),
                tr("brand_model_required")
            );

            return;
        }

        const Category newCategory =
            category.currentIndex() == 0 ? Category::Men :
            category.currentIndex() == 1 ? Category::Women :
            Category::Other;

        // 5. Reacquire the product and update its fields.
        watch = catalog_.findByIdMutable(id);

        if (!watch) {
            QMessageBox::warning(
                this,
                tr("select_product"),
                tr("select_row")
            );

            return;
        }

        watch->setBrand(ss(newBrand));
        watch->setName(ss(newModel));
        watch->setPrice(priceBox.value());
        watch->setCategory(newCategory);
        watch->setStock(stock.value());

        // 6. Save the updated inventory to inventory.txt.
        catalog_.saveToFile();

        // 7. Refresh the catalog and cart display.
        refreshAll();

        // Restore selection to the product that was just edited.
        for (int newRow = 0;
             newRow < catalogTable_->rowCount();
             ++newRow) {

            if (catalogTable_->item(newRow, 0) &&
                catalogTable_->item(newRow, 0)->text().toInt() == id) {

                catalogTable_->selectRow(newRow);
                break;
            }
        }

        QMessageBox::information(
            this,
            tr("product_updated"),
            tr("product_update_saved")
        );
    }


    // ---------------------------------------------------------
    // ADMIN: DELETE PRODUCT
    // ---------------------------------------------------------

    void deleteProduct() {
        const int id = selectedId(catalogTable_);

        if (id < 0) {
            QMessageBox::information(
                this,
                tr("select_product"),
                tr("select_row")
            );

            return;
        }

        if (QMessageBox::question(
                this,
                tr("delete_product_title"),
                tr("delete_id") + " " +
                QString::number(id) + "?") != QMessageBox::Yes) {

            return;
        }

        if (catalog_.deleteWatch(id))
            refreshAll();
    }


    // ---------------------------------------------------------
    // ADMIN: USER MANAGEMENT
    // ---------------------------------------------------------

    void manageUsers() {
        bool ok = false;

        const QStringList actions = {
            tr("add_account"),
            tr("remove_account"),
            tr("close")
        };

        const QString action = QInputDialog::getItem(
            this,
            tr("user_management"),
            tr("action"),
            actions,
            0,
            false,
            &ok
        );

        if (!ok || action == tr("close"))
            return;

        if (action == tr("add_account")) {
            QDialog dialog(this);

            dialog.setWindowTitle(tr("add_account"));

            auto* form = new QFormLayout(&dialog);

            QLineEdit username, password;

            password.setEchoMode(QLineEdit::Password);

            QComboBox role;
            role.addItems({
                tr("customer"),
                tr("admin")
            });

            form->addRow(tr("username"), &username);
            form->addRow(tr("password"), &password);
            form->addRow(tr("role"), &role);

            auto* buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok |
                QDialogButtonBox::Cancel
            );

            buttons->button(
                QDialogButtonBox::Ok
            )->setText(tr("ok"));

            buttons->button(
                QDialogButtonBox::Cancel
            )->setText(tr("cancel"));

            form->addRow(buttons);

            connect(
                buttons,
                &QDialogButtonBox::accepted,
                &dialog,
                &QDialog::accept
            );

            connect(
                buttons,
                &QDialogButtonBox::rejected,
                &dialog,
                &QDialog::reject
            );

            if (dialog.exec() != QDialog::Accepted)
                return;

            if (username.text().trimmed().isEmpty() ||
                password.text().isEmpty()) {
                return;
            }

            const Role newRole =
                role.currentIndex() == 1
                    ? Role::Admin
                    : Role::Customer;

            if (!User::addUser(
                    ss(username.text().trimmed()),
                    ss(password.text()),
                    newRole
                )) {

                QMessageBox::warning(
                    this,
                    tr("user_exists"),
                    tr("username_registered")
                );

            } else {

                QMessageBox::information(
                    this,
                    tr("user_added"),
                    tr("account_saved")
                );
            }

        } else {
            QStringList names;

            for (const auto& user : User::listUsers()) {
                names << qs(user.getUsername());
            }

            const QString name = QInputDialog::getItem(
                this,
                tr("remove_account"),
                tr("choose_account"),
                names,
                0,
                false,
                &ok
            );

            if (!ok || name.isEmpty())
                return;

            std::string reason;

            if (!User::removeUser(ss(name), reason)) {
                QMessageBox::warning(
                    this,
                    tr("cannot_remove"),
                    qs(reason)
                );
            } else {
                QMessageBox::information(
                    this,
                    tr("account_removed"),
                    tr("account_removed_msg")
                );
            }
        }
    }
};


// -------------------------------------------------------------
// APPLICATION ENTRY POINT
// -------------------------------------------------------------

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setApplicationName("TEAM CHRONOS");

    std::string username;
    Role role = Role::Customer;
    bool authenticated = false;

    for (int attempt = 0;
         attempt < 3 && !authenticated;
         ++attempt) {

        LoginDialog login;

        if (login.exec() != QDialog::Accepted)
            return 0;

        const User* user = User::findUser(
            ss(login.username()),
            ss(login.password())
        );

        if (user) {
            username = user->getUsername();
            role = user->getRole();
            authenticated = true;
        } else {
            QMessageBox::warning(
                nullptr,
                "Login failed",
                "Invalid username or password.\nAttempts remaining: " +
                QString::number(2 - attempt)
            );
        }
    }

    if (!authenticated) {
        QMessageBox::critical(
            nullptr,
            "Login",
            "Too many failed attempts."
        );

        return 0;
    }

    StoreWindow window(username, role);

    window.show();

    return app.exec();
}