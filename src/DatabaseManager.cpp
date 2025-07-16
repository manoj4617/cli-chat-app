#include "DatabaseManager.hpp"

DatabaseManager::DatabaseManager(const std::string& db_path){
    try {
        // Use flags to create the database if it doesn't exist
        int flags = SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE;
        db_ = std::make_unique<SQLite::Database>(db_path, flags);
        is_initialized_ = true; // If we reach here, the file was opened/created successfully
        std::cout << "[INFO] Database connection established to: " << db_path << std::endl;
    } catch (const SQLite::Exception& e) {
        std::cerr << "[ERROR] Failed to open database '" << db_path << "': " << e.what() << std::endl;
        db_ = nullptr; // Ensure db_ is null on failure
        is_initialized_ = false;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] An unexpected error occurred opening the database: " << e.what() << std::endl;
        db_ = nullptr;
        is_initialized_ = false;
    }
}

bool DatabaseManager::is_valid() const {
    return is_initialized_ && (db_ != nullptr);
}