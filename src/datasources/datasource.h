#ifndef datasource
#define datasource

// Represents a datasource.
enum class DatasourceID
{
    Deribit,
};

// An action on an offer, an offer can either be added, updated or removed.
enum class OfferAction
{
    Add,
    Update,
    Remove,
};

// Represents an offer in the orderbook.
typedef struct
{
    double price;
    double quantity;
} Offer;

// Represents an offer in the orderbook.
typedef struct
{
    OfferAction action;
    Offer offer;
} OfferChange;

// Represents an orderbook snapshot.
typedef struct
{
    std::vector<Offer> bids;
    std::vector<Offer> asks;
} BidAskSnapshot;

// Represents an orderbook delta update.
typedef struct
{
    std::vector<OfferChange> bids;
    std::vector<OfferChange> asks;
} BidAskDelta;

#endif // datasource
