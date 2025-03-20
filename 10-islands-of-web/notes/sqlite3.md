# Stuff about sqlite3

## Performance

- https://www.powersync.com/blog/sqlite-optimizations-for-ultra-high-performance

## JSON

- https://www.beekeeperstudio.io/blog/sqlite-json

## Examples:

Assume this json data is stored as `OriginData` in `movies` table.

```js
{
  "Title": "Blade Runner",
  "Year": "1982",
  "Rated": "R",
  "Released": "25 Jun 1982",
  "Runtime": "117 min",
  "Genre": "Action, Drama, Sci-Fi",
  "Director": "Ridley Scott",
  "Writer": "Hampton Fancher, David Webb Peoples, Philip K. Dick",
  "Actors": "Harrison Ford, Rutger Hauer, Sean Young",
  "Plot": "A blade runner must pursue and terminate four replicants who stole a ship in space and have returned to Earth to find their creator.",
  "Language": "English, German, Cantonese, Japanese, Hungarian, Arabic, Korean",
  "Country": "United States, United Kingdom",
  "Awards": "Nominated for 2 Oscars. 13 wins & 22 nominations total",
  "Poster": "https://m.media-amazon.com/images/M/MV5BOWQ4YTBmNTQtMDYxMC00NGFjLTkwOGQtNzdhNmY1Nzc1MzUxXkEyXkFqcGc@._V1_SX300.jpg",
  "Ratings": [
    {
      "Source": "Internet Movie Database",
      "Value": "8.1/10"
    },
    {
      "Source": "Rotten Tomatoes",
      "Value": "89%"
    },
    {
      "Source": "Metacritic",
      "Value": "84/100"
    }
  ],
  "Metascore": "84",
  "imdbRating": "8.1",
  "imdbVotes": "834,912",
  "imdbID": "tt0083658",
  "Type": "movie",
  "DVD": "N/A",
  "BoxOffice": "$32,914,489",
  "Production": "N/A",
  "Website": "N/A",
  "Response": "True"
}
```

Then these is how you can retrieve various fields out of it as below. It is
instructive to read the documentation at https://www.sqlite.org/json1.html

```bash
# extract ratings field out of the object
[sqlite> select OriginData ->> '$.Ratings' as ratings from movies;
ratings = [{"Source":"Internet Movie Database","Value":"8.1/10"},{"Source":"Rotten Tomatoes","Value":"89%"},{"Source":"Metacritic","Value":"84/100"}]

ratings = [{"Source":"Internet Movie Database","Value":"9.2/10"},{"Source":"Rotten Tomatoes","Value":"97%"},{"Source":"Metacritic","Value":"100/100"}]

ratings = [{"Source":"Internet Movie Database","Value":"8.9/10"},{"Source":"Rotten Tomatoes","Value":"92%"},{"Source":"Metacritic","Value":"95/100"}]

ratings = [{"Source":"Internet Movie Database","Value":"8.3/10"},{"Source":"Rotten Tomatoes","Value":"92%"},{"Source":"Metacritic","Value":"84/100"}]
```

```bash
# extract imdb ratings out of ratings
[sqlite> select OriginData ->> '$.Ratings' ->> 0 as imdb_ratings from movies;
imdb_ratings = {"Source":"Internet Movie Database","Value":"8.1/10"}

imdb_ratings = {"Source":"Internet Movie Database","Value":"9.2/10"}

imdb_ratings = {"Source":"Internet Movie Database","Value":"8.9/10"}

imdb_ratings = {"Source":"Internet Movie Database","Value":"8.3/10"}
```

```bash
# extract title and imdb rating
[sqlite> select json_extract(OriginData, '$.Title', '$.Ratings[0].Value') from movies;
json_extract(OriginData, '$.Title', '$.Ratings[0].Value') = ["Blade Runner","8.1/10"]

json_extract(OriginData, '$.Title', '$.Ratings[0].Value') = ["The Godfather","9.2/10"]

json_extract(OriginData, '$.Title', '$.Ratings[0].Value') = ["Pulp Fiction","8.9/10"]

json_extract(OriginData, '$.Title', '$.Ratings[0].Value') = ["2001: A Space Odyssey","8.3/10"]
```
