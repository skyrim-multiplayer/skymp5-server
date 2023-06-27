import React from 'react';

const D100 = (props: { className?: string; onClick?: () => void }) => {
  return (
    <div className="dice-wrapper">
      <svg
        width="48"
        onClick={props.onClick ? props.onClick : () => {}}
        className={`dice icon-d100 ${props.className}`}
        height="48"
        viewBox="0 0 48 48"
        fill="none"
        xmlns="http://www.w3.org/2000/svg"
      >
        <path
          d="M36.4499 12.05L30.8999 8.30005L24.3999 7.05005L17.7499 8.30005L12.1999 12.05L8.4499 17.65L7.1499 24.2001L8.4499 30.65L12.1999 36.3L17.7499 40L24.3999 41.25L30.8999 40L36.4499 36.3L40.0999 30.65L41.4499 24.2001L40.0999 17.65L36.4499 12.05ZM24.3999 9.40005L29.4499 10.35L28.1999 14.15L26.2999 14.75L23.7499 12.55L24.3999 9.40005ZM25.9999 15.3L25.5999 16.5L26.2499 16.55L26.5499 15.35L28.6499 14.7L31.3999 16.4L31.9499 18.6L32.4999 19.2L35.1499 20.6L35.7999 23.15L35.3999 23.9501L35.8499 24.6L36.4999 23.4501L39.0499 24.1L38.0999 29.15L35.7499 28.35V28.3L35.0499 28.9501L33.1999 31.5H30.5499L30.1999 30.55L29.4499 30.85L29.9999 31.85L28.3999 34.55L24.9499 35.3L22.8499 33.7001L23.0999 32.5L22.3499 32.2001L22.0999 33.55L19.2499 34.05L16.9999 32.3L15.8999 28.9501H15.8499L13.9499 27.6L13.3999 24.65L13.9999 23.9501L13.6499 23.4501L12.7999 24.4501L9.6499 23.6L10.5499 19.05L14.1499 19.7L14.2999 20.35L14.8999 20L14.6999 19.1L16.1499 16.8L19.4499 16.5L20.2499 17.35L20.7499 17.05L19.9999 16L20.9999 13.55L23.6499 13.15L25.9999 15.3ZM22.9499 12.55L20.6499 13.05L18.5999 10.45L23.6499 9.55005L22.9499 12.55ZM34.1999 13.35L31.3999 15.5L28.8499 14L29.9999 10.45L34.1999 13.35ZM31.8999 15.95L34.7499 13.65L37.5499 17.8L35.1499 19.75L32.6999 18.6L31.8999 15.95ZM38.9499 23.4501L36.4999 22.65L35.7999 20.05L37.8999 18.45L38.9499 23.4501ZM19.5499 15.55L15.7499 16.2L13.8499 13.65L18.1999 10.75L20.2499 13.6L19.5499 15.55ZM10.6499 18.45L13.5499 14.2L15.3999 16.75L14.0499 19L10.6499 18.45ZM13.3999 27.9L10.6499 29.8L9.5999 24.55L12.9999 25.15L13.3999 27.9ZM10.9499 30.2001L13.8499 28.3L15.4499 29.5L16.4499 32.6L13.8499 34.6L10.9499 30.2001ZM16.9999 33.1L19.1499 34.7001L18.5999 37.75L14.4999 34.9501L16.9999 33.1ZM19.1499 37.85L19.7999 34.6L22.3499 34.05L24.8999 36L24.3999 38.9001L19.1499 37.85ZM35.6999 29.15L37.8999 29.8L35.1499 34L33.9499 31.55L35.6999 29.15ZM33.5499 32.05L34.7499 34.6L30.4999 37.5L29.2499 34.55L30.3999 32.2001L33.5499 32.05ZM25.4999 35.8L28.6999 35.2001L29.9999 37.75L24.9499 38.75L25.4999 35.8ZM17.4499 21.6C17.4166 22.0334 17.3999 22.6 17.3999 23.3V29.4H18.9999V20H17.6499L14.8999 22.15L15.6499 23.1C15.7832 23 15.9166 22.9334 16.0499 22.9C16.3166 22.5667 16.5666 22.35 16.7999 22.25C16.9332 22.0834 17.0332 21.9834 17.0999 21.95C17.2999 21.7834 17.4166 21.6667 17.4499 21.6ZM28.3999 21C27.8666 21.8 27.5999 23.0334 27.5999 24.7001C27.5999 26.2667 27.8666 27.4667 28.3999 28.3C28.9332 29.1 29.7332 29.5 30.7999 29.5C31.8999 29.5 32.6999 29.1167 33.1999 28.35C33.5666 27.85 33.8166 27.1667 33.9499 26.3C33.9832 26.0334 34.0166 25.7501 34.0499 25.4501C34.0499 25.2167 34.0499 24.9667 34.0499 24.7001C34.0499 23.1001 33.7666 21.9 33.1999 21.1C32.7999 20.4334 32.2666 20.0334 31.5999 19.9C31.5332 19.8667 31.4832 19.85 31.4499 19.85C31.2499 19.8167 31.0332 19.8 30.7999 19.8C29.7332 19.8 28.9332 20.2 28.3999 21ZM29.1499 24.7001C29.1499 23.4334 29.2832 22.5167 29.5499 21.95C29.8166 21.4167 30.2332 21.15 30.7999 21.15C31.3666 21.15 31.7832 21.4167 32.0499 21.95C32.3499 22.5167 32.4999 23.4334 32.4999 24.7001C32.4999 25.9667 32.3499 26.8834 32.0499 27.4501C31.7832 28.0167 31.3666 28.3 30.7999 28.3C30.2332 28.3 29.8166 28.0334 29.5499 27.5C29.2832 26.9334 29.1499 26.0001 29.1499 24.7001ZM21.0999 21C20.5666 21.8 20.2999 23.0334 20.2999 24.7001C20.2999 26.2667 20.5666 27.4667 21.0999 28.3C21.6666 29.1 22.4666 29.5 23.4999 29.5C24.6332 29.5 25.4666 29.1167 25.9999 28.35C26.4999 27.5167 26.7499 26.3001 26.7499 24.7001C26.7499 23.1001 26.4832 21.9 25.9499 21.1C25.3832 20.2334 24.5666 19.8 23.4999 19.8C22.4332 19.8 21.6332 20.2 21.0999 21ZM24.7999 21.95C25.0666 22.5167 25.1999 23.4334 25.1999 24.7001C25.1999 25.9667 25.0666 26.8834 24.7999 27.4501C24.5332 28.0167 24.0999 28.3 23.4999 28.3C22.8999 28.3 22.4832 28.0334 22.2499 27.5C21.9832 26.9334 21.8499 26.0001 21.8499 24.7001C21.8499 23.4334 21.9832 22.5167 22.2499 21.95C22.4832 21.4167 22.8999 21.15 23.4999 21.15C24.0999 21.15 24.5332 21.4167 24.7999 21.95Z"
          fill="black"
        />
        <path
          d="M32.5001 24.7C32.5001 23.4334 32.3501 22.5167 32.0501 21.95C31.7834 21.4167 31.3668 21.15 30.8001 21.15C30.2334 21.15 29.8168 21.4167 29.5501 21.95C29.2834 22.5167 29.1501 23.4334 29.1501 24.7C29.1501 26 29.2834 26.9334 29.5501 27.5C29.8168 28.0334 30.2334 28.3 30.8001 28.3C31.3668 28.3 31.7834 28.0167 32.0501 27.45C32.3501 26.8834 32.5001 25.9667 32.5001 24.7ZM22.2501 21.95C21.9834 22.5167 21.8501 23.4334 21.8501 24.7C21.8501 26 21.9834 26.9334 22.2501 27.5C22.4834 28.0334 22.9001 28.3 23.5001 28.3C24.1001 28.3 24.5334 28.0167 24.8001 27.45C25.0668 26.8834 25.2001 25.9667 25.2001 24.7C25.2001 23.4334 25.0668 22.5167 24.8001 21.95C24.5334 21.4167 24.1001 21.15 23.5001 21.15C22.9001 21.15 22.4834 21.4167 22.2501 21.95Z"
          className="to-color"
        />
        <g clipPath="url(#clip0_12_373)">
          <path
            d="M28.7001 35.2L25.5001 35.8L24.9501 38.75L30.0001 37.75L28.7001 35.2ZM35.1501 34L37.9001 29.8L35.7001 29.15L33.9501 31.55L35.1501 34ZM33.5501 32.05L30.4001 32.2L29.2501 34.55L30.5001 37.5L34.7501 34.6L33.5501 32.05ZM19.8001 34.6L19.1501 37.85L24.4001 38.9L24.9001 36L22.3501 34.05L19.8001 34.6ZM19.1501 34.7L17.0001 33.1L14.5001 34.95L18.6001 37.75L19.1501 34.7ZM13.8501 28.3L10.9501 30.2L13.8501 34.6L16.4501 32.6L15.4501 29.5L13.8501 28.3ZM10.6501 29.8L13.4001 27.9L13.0001 25.15L9.6001 24.55L10.6501 29.8ZM15.4001 16.75L13.5501 14.2L10.6501 18.45L14.0501 19L15.4001 16.75ZM15.7501 16.2L19.5501 15.55L20.2501 13.6L18.2001 10.75L13.8501 13.65L15.7501 16.2ZM35.8001 20.05L36.5001 22.65L38.9501 23.45L37.9001 18.45L35.8001 20.05ZM31.4001 15.5L34.2001 13.35L30.0001 10.45L28.8501 14L31.4001 15.5ZM34.7501 13.65L31.9001 15.95L32.7001 18.6L35.1501 19.75L37.5501 17.8L34.7501 13.65ZM20.6501 13.05L22.9501 12.55L23.6501 9.55002L18.6001 10.45L20.6501 13.05ZM29.4501 10.35L24.4001 9.40002L23.7501 12.55L26.3001 14.75L28.2001 14.15L29.4501 10.35Z"
            className="to-color"
          />
          <path
            fillRule="evenodd"
            clipRule="evenodd"
            d="M25.9999 15.3L25.5999 16.5L26.2499 16.55L26.5499 15.35L28.6499 14.7L31.3999 16.4L31.9499 18.6L32.4999 19.2L35.1499 20.6L35.7999 23.15L35.3999 23.95L35.8499 24.6L36.4999 23.45L39.0499 24.1L38.0999 29.15L35.7499 28.35V28.3L35.0499 28.95L33.1999 31.5H30.5499L30.1999 30.55L29.4499 30.85L29.9999 31.85L28.3999 34.55L24.9499 35.3L22.8499 33.7L23.0999 32.5L22.3499 32.2L22.0999 33.55L19.2499 34.05L16.9999 32.3L15.8999 28.95H15.8499L13.9499 27.6L13.3999 24.65L13.9999 23.95L13.6499 23.45L12.7999 24.45L9.6499 23.6L10.5499 19.05L14.1499 19.7L14.2999 20.35L14.8999 20L14.6999 19.1L16.1499 16.8L19.4499 16.5L20.2499 17.35L20.7499 17.05L19.9999 16L20.9999 13.55L23.6499 13.15L25.9999 15.3ZM17.3999 23.3C17.3999 22.6 17.4166 22.0334 17.4499 21.6C17.4166 21.6667 17.2999 21.7834 17.0999 21.95C17.0332 21.9834 16.9332 22.0834 16.7999 22.25C16.5666 22.35 16.3166 22.5667 16.0499 22.9C15.9166 22.9334 15.7832 23 15.6499 23.1L14.8999 22.15L17.6499 20H18.9999V29.4H17.3999V23.3ZM27.5999 24.7C27.5999 23.0334 27.8666 21.8 28.3999 21C28.9332 20.2 29.7332 19.8 30.7999 19.8C31.0332 19.8 31.2499 19.8167 31.4499 19.85C31.4832 19.85 31.5332 19.8667 31.5999 19.9C32.2666 20.0334 32.7999 20.4334 33.1999 21.1C33.7666 21.9 34.0499 23.1 34.0499 24.7V25.45C34.0166 25.75 33.9832 26.0334 33.9499 26.3C33.8166 27.1667 33.5666 27.85 33.1999 28.35C32.6999 29.1167 31.8999 29.5 30.7999 29.5C29.7332 29.5 28.9332 29.1 28.3999 28.3C27.8666 27.4667 27.5999 26.2667 27.5999 24.7ZM20.2999 24.7C20.2999 23.0334 20.5666 21.8 21.0999 21C21.6332 20.2 22.4332 19.8 23.4999 19.8C24.5666 19.8 25.3832 20.2334 25.9499 21.1C26.4832 21.9 26.7499 23.1 26.7499 24.7C26.7499 26.3 26.4999 27.5167 25.9999 28.35C25.4666 29.1167 24.6332 29.5 23.4999 29.5C22.4666 29.5 21.6666 29.1 21.0999 28.3C20.5666 27.4667 20.2999 26.2667 20.2999 24.7Z"
            className="to-color"
          />
        </g>
        <defs>
          <clipPath id="clip0_12_373">
            <rect width="48" height="48" fill="white" />
          </clipPath>
        </defs>
      </svg>
    </div>
  );
};

export default D100;
